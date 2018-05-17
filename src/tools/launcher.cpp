#include <gtkmm.h>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/program_options.hpp>
#include "../common/config_files.hpp"

using namespace std::string_literals;

bool file_exists(const char* path, unsigned int mode = F_OK)
{
    return access(path, mode) == 0;
}

bool executable_exists(std::string_view dir, std::string_view executable)
{
    if (dir.size() + executable.size() + 1 >= PATH_MAX) {
        std::cerr << "Error: requested path longer than PATH_MAX" << std::endl;
        return false;
    }
    char path_buf[PATH_MAX];
    auto ptr = std::copy(dir.begin(), dir.end(), path_buf);
    *ptr = '/';
    ptr = std::copy(executable.begin(), executable.end(), ptr + 1);
    *ptr = '\0';

    return file_exists(path_buf, F_OK | X_OK);
}

auto file_filter_for_extension(std::string_view extension)
{
    auto filter = Gtk::FileFilter::create();
    auto pattern = "*."s;
    pattern += extension;
    filter->set_name(std::string(extension));
    filter->add_pattern(pattern);

    return filter;
}

char * color_string(Gtk::ColorButton &button)
{
    char* buffer = new char[2 * 4 + 2]; // 2 per channel, plus #, plus null byte
    auto color = button.get_rgba();

    // Note: Gdk stores its RGBA values in range 0..66535 instead of 0..255, so need to scale.
    sprintf(buffer, "#%02x%02x%02x%02x", color.get_red_u() >> 8,
            color.get_green_u() >> 8, color.get_blue_u() >> 8, color.get_alpha_u() >> 8);

    return buffer;
}

/**
 * Wrap string into a dynamically allocated c-string.
 *
 * @param str
 * @return
 */
char* wrap_string(std::string_view str) {
    if (str.empty()) {
        return nullptr;
    }

    auto wrapper = new char[str.size() + 1];
    str.copy(wrapper, str.size());
    wrapper[str.size()] = '\0';

    return wrapper;
}


void float_parameter(std::vector<char *> &argv, std::string_view flag, double value)
{
    char buffer[100];
    argv.push_back(wrap_string(flag));
    std::sprintf(buffer, "%f", value);
    argv.push_back(wrap_string(buffer));
}

void int_parameter(std::vector<char*> &argv, std::string_view flag, int value)
{
    char buffer[100];
    argv.push_back(wrap_string(flag));
    std::sprintf(buffer, "%d", value);
    argv.push_back(wrap_string(buffer));
}

void set_default_path(Gtk::FileChooserButton& chooser, const char* path)
{
    if (file_exists(path)) {
        chooser.set_filename(path);
    }
}

class Launcher : public Gtk::Window {
public:
    Launcher();
    ~Launcher() override = default;

private:
    int rows;

    Gtk::Box box;
    Gtk::ScrolledWindow scrolledWindow;
    Gtk::Grid grid;
    Gtk::FileChooserButton fmriChooser;
    Gtk::FileChooserButton modelChooser;
    Gtk::FileChooserButton weightsChooser;
    Gtk::FileChooserButton labelChooser;
    Gtk::FileChooserButton meansChooser;
    Gtk::FileChooserButton inputChooser;
    Gtk::ColorButton pathColor;
    Gtk::ColorButton bgColor;
    Gtk::ColorButton neutralColor;
    Gtk::ColorButton positiveColor;
    Gtk::ColorButton negativeColor;
    Gtk::Scale layerDistance;
    Gtk::Scale layerTransparency;
    Gtk::Scale interactionTransparency;
    Gtk::SpinButton interactionLimit;
    Gtk::Switch brainSwitch;
    Gtk::Button startButton;

    void start();
    bool hasFile(const Gtk::FileChooserButton& fileChooser, const std::string& error);
    std::vector<std::string> getInputFiles();
    Gtk::Label* getManagedLabel(const std::string& contents);
    void findExecutable();
    void addRowWithLabel(const std::string& label, Gtk::Widget& widget);
    void addHeaderRow(const std::string& header);

    void setDefaultsFromFile(const char* file);

    void setPath(Gtk::FileChooserButton &target, const boost::program_options::variables_map &vm, const char *name);

    void setColor(Gtk::ColorButton &target, const boost::program_options::variables_map &vm, const char *name);

    void setSlider(Gtk::Scale &target, const boost::program_options::variables_map &vm, const char *name);
};

Launcher::Launcher()
        :
        Gtk::Window(),
        rows(0),
        box(Gtk::Orientation::ORIENTATION_VERTICAL),
        fmriChooser("Select FMRI executable"),
        modelChooser("Select caffe model prototxt"),
        weightsChooser("Select caffe model weights"),
        labelChooser("Select label text file"),
        meansChooser("Select means file"),
        inputChooser("Select input directory", Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER),
        pathColor(Gdk::RGBA("rgba(255, 255, 255, 0.1)")),
        bgColor(Gdk::RGBA("rgba(0, 0, 0, 0)")),
        neutralColor(Gdk::RGBA("rgba(255, 255, 255, 1)")),
        positiveColor(Gdk::RGBA("rgba(0, 0, 255, 1)")),
        negativeColor(Gdk::RGBA("rgba(255, 0, 0, 1)")),
        layerDistance(Gtk::Adjustment::create(10, 0, 100, 0, 0.1, 0)),
        layerTransparency(Gtk::Adjustment::create(1, 0, 1, 0.0, 1.f / 256)),
        interactionTransparency(Gtk::Adjustment::create(1, 0, 1, 0.0, 1.f / 256)),
        interactionLimit(Gtk::Adjustment::create(10000, 1, std::numeric_limits<int>::max()), 10000),
        startButton("Start FMRI")
{
    set_default_size(480, 320);
    add(box);
    box.add(scrolledWindow);
    scrolledWindow.set_vexpand(true);
    box.add(startButton);
    scrolledWindow.add(grid);

    // Configure all widgets
    fmriChooser.set_hexpand(true);
    modelChooser.add_filter(file_filter_for_extension("prototxt"));
    weightsChooser.add_filter(file_filter_for_extension("caffemodel"));
    labelChooser.add_filter(file_filter_for_extension("txt"));
    meansChooser.add_filter(file_filter_for_extension("binaryproto"));
    pathColor.set_use_alpha(true);
    brainSwitch.set_state(false);

    // Set the default paths if called from the expected place
    findExecutable();
    set_default_path(modelChooser, "../data/models/alexnet/model-dedup.prototxt");
    set_default_path(weightsChooser, "../data/models/alexnet/bvlc_alexnet.caffemodel");
    set_default_path(labelChooser, "../data/ilsvrc12/synset_words.txt");
    set_default_path(meansChooser, "../data/ilsvrc12/imagenet_mean.binaryproto");
    set_default_path(inputChooser, "../data/samples");

    // Configure grid display options
    grid.set_row_spacing(2);
    grid.set_column_spacing(2);

    // Attach widgets to the grid
    addRowWithLabel("FMRI executable", fmriChooser);
    addRowWithLabel("Model", modelChooser);
    addRowWithLabel("Weights", weightsChooser);
    addRowWithLabel("Labels (optional)", labelChooser);
    addRowWithLabel("Input directory", inputChooser);
    addRowWithLabel("Means (optional)", meansChooser);
    addHeaderRow("Color settings");
    addRowWithLabel("Path color", pathColor);
    addRowWithLabel("Background color", bgColor);
    addRowWithLabel("Neutral color", neutralColor);
    addRowWithLabel("Positive color", positiveColor);
    addRowWithLabel("Negative color", negativeColor);
    addHeaderRow("Misc settings");
    addRowWithLabel("Layer distance", layerDistance);
    addRowWithLabel("Layer transparency", layerTransparency);
    addRowWithLabel("Interaction transparency", interactionTransparency);
    addRowWithLabel("Interaction limit", interactionLimit);
    addRowWithLabel("Brain mode", brainSwitch);

    setDefaultsFromFile(fmri::MAIN_CONFIG_FILE);

    startButton.signal_clicked().connect(sigc::mem_fun(*this, &Launcher::start));
    //grid.attach_next_to(startButton, Gtk::PositionType::POS_BOTTOM, 2, 1);
    show_all_children(true);
}

void Launcher::start()
{
    if (!hasFile(fmriChooser, "Need executable") ||
            !hasFile(modelChooser, "Need model file") ||
            !hasFile(weightsChooser, "Need weights file") ||
            !hasFile(inputChooser, "Need input folder")) {
        return;
    }

    auto executable = fmriChooser.get_file()->get_path();
    auto network = modelChooser.get_file()->get_path();
    auto weights = weightsChooser.get_file()->get_path();
    auto inputs = getInputFiles();

    if (inputs.empty()) {
        Gtk::MessageDialog dialog(*this, "No inputs in folder!");
        dialog.run();
        return;
    }

    std::vector<char*> argv = {
            wrap_string(executable),
            wrap_string("-n"),
            wrap_string(network),
            wrap_string("-w"),
            wrap_string(weights),
            wrap_string("-p"),
            color_string(pathColor),
            wrap_string("--background-color"),
            color_string(bgColor),
            wrap_string("--neutral-color"),
            color_string(neutralColor),
            wrap_string("--positive-color"),
            color_string(positiveColor),
            wrap_string("--negative-color"),
            color_string(negativeColor)
    };

    float_parameter(argv, "--layer-opacity", layerTransparency.get_value());
    float_parameter(argv, "--interaction-opacity", interactionTransparency.get_value());
    float_parameter(argv, "--layer-distance", layerDistance.get_value());
    int_parameter(argv, "--interaction-limit", interactionLimit.get_value());

    if (labelChooser.get_file()) {
        argv.push_back(wrap_string("-l"));
        argv.push_back(wrap_string(labelChooser.get_file()->get_path()));
    }

    if (meansChooser.get_file()) {
        argv.push_back(wrap_string("-m"));
        argv.push_back(wrap_string(meansChooser.get_file()->get_path()));
    }

    if (brainSwitch.get_state()) {
        argv.push_back(wrap_string("-b"));
    }

    std::transform(inputs.begin(), inputs.end(), std::back_inserter(argv),
            [](const auto& x) { return wrap_string(x); });

    argv.push_back(nullptr);

    execv(executable.c_str(), argv.data());

    // Discard all allocated memory.
    std::for_each(argv.begin(), argv.end(), std::default_delete<char[]>());
    Gtk::MessageDialog dialog(*this, "Failed to start for unknown reasons.");
    dialog.run();
}

bool Launcher::hasFile(const Gtk::FileChooserButton& fileChooser, const std::string& error)
{
    if (!fileChooser.get_file()) {
        Gtk::MessageDialog dialog(*this, error);
        dialog.run();
        return false;
    }

    return true;
}

std::vector<std::string> Launcher::getInputFiles()
{
    using namespace boost::filesystem;

    std::vector<std::string> result;

    auto folder = path(inputChooser.get_file()->get_path());

    directory_iterator end_itr;
    for (directory_iterator itr(folder); itr!=end_itr; ++itr) {
        if (!is_regular(itr->status())) {
            continue;
        }

        result.push_back(itr->path().string());
    }

    std::sort(result.begin(), result.end());

    return result;
}

Gtk::Label* Launcher::getManagedLabel(const std::string& contents)
{
    auto label = new Gtk::Label(contents);
    Gtk::manage(label);

    return label;
}

void Launcher::findExecutable()
{
    if (executable_exists(".", "fmri")) {
        fmriChooser.set_filename("fmri");
        return;
    }

    std::string_view env_path = std::getenv("PATH");
    std::string_view::size_type offset = 0;
    while (offset != std::string_view::npos) {
        auto limit = env_path.find(":");
        auto component = env_path.substr(offset, limit);
        if (limit != std::string_view::npos) {
            offset = limit + 1;
        } else {
            offset = limit;
        }
        if (!component.empty() && executable_exists(component, "fmri")) {
            fmriChooser.set_filename(std::string(component) + "/fmri");
            return;
        }
    }
}

void Launcher::addRowWithLabel(const std::string &label, Gtk::Widget &widget)
{
    int currentRow = rows++;
    grid.attach(widget, 1, currentRow, 1, 1);
    grid.attach_next_to(*getManagedLabel(label), widget, Gtk::PositionType::POS_LEFT, 1, 1);
}

void Launcher::addHeaderRow(const std::string &header)
{
    int currentRow = rows++;
    auto label = getManagedLabel(header);
    label->set_markup("<b>" + header + "</b>");
    grid.attach(*label, 0, currentRow, 2, 1);
}

void Launcher::setDefaultsFromFile(const char *file)
{
    using namespace boost::program_options;

    auto configFile = fmri::get_xdg_config(file);
    if (!configFile.good()) {
        return;
    }

    options_description options;
    options.add_options()
            ("weights", value<std::string>())
            ("network", value<std::string>())
            ("labels", value<std::string>())
            ("means", value<std::string>())
            ("path-color", value<std::string>())
            ("layer-opacity", value<float>())
            ("interaction-opacity", value<float>())
            ("layer-distance", value<float>())
            ("interaction-limit", value<int>())
            ("neutral-color", value<std::string>())
            ("positive-color", value<std::string>())
            ("negative-color", value<std::string>())
            ("background-color", value<std::string>());

    variables_map vm;
    store(parse_config_file(configFile, options, true), vm);

    setPath(weightsChooser, vm, "weights");
    setPath(modelChooser, vm, "network");
    setPath(labelChooser, vm, "labels");
    setPath(meansChooser, vm, "means");

    setColor(pathColor, vm, "path-color");
    setColor(neutralColor, vm, "neutral-color");
    setColor(positiveColor, vm, "positive-color");
    setColor(negativeColor, vm, "negative-color");

    setSlider(layerDistance, vm, "layer-distance");
    setSlider(layerTransparency, vm, "layer-opacity");
    setSlider(interactionTransparency, vm, "interaction-opacity");

    if (vm.count("interaction-limit")) {
        interactionLimit.set_value(vm["interaction-limit"].as<int>());
    }
}

void
Launcher::setPath(Gtk::FileChooserButton &target, const boost::program_options::variables_map &vm, const char *name)
{
    if (vm.count(name)) {
        target.set_filename(vm[name].as<std::string>());
    }
}

void Launcher::setColor(Gtk::ColorButton &target, const boost::program_options::variables_map &vm, const char *name)
{
    if (vm.count(name)) {
        auto res = vm[name].as<std::string>();
        if (std::isxdigit(res[0])) {
            res = "#"s + res;
        }
        target.set_rgba(Gdk::RGBA(res));
    }
}

void Launcher::setSlider(Gtk::Scale &target, const boost::program_options::variables_map &vm, const char *name)
{
    if (vm.count(name)) {
        target.set_value(vm[name].as<float>());
    }

}

int main(int argc, char** argv)
{
    auto app = Gtk::Application::create(argc, argv);
    Launcher launcher;
    return app->run(launcher);
}
