#include <gtkmm.h>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

bool executable_exists(std::string_view dir, std::string_view executable)
{
    std::string executable_path(dir);
    executable_path += '/';
    executable_path += executable;

    return access(executable_path.c_str(), F_OK | X_OK) == 0;
}

class Launcher : public Gtk::Window {
public:
    Launcher();
    ~Launcher() override = default;

private:
    Gtk::Box box;
    Gtk::FileChooserButton fmriChooser;
    Gtk::FileChooserButton modelChooser;
    Gtk::FileChooserButton weightsChooser;
    Gtk::FileChooserButton inputChooser;
    Gtk::Button startButton;

    void start();
    bool hasFile(const Gtk::FileChooserButton& fileChooser, const std::string& error);
    std::vector<std::string> getInputFiles();
    Gtk::Label* getManagedLabel(const std::string& contents);
    void findExecutable();
};

Launcher::Launcher()
        :
        Gtk::Window(),
        box(Gtk::Orientation::ORIENTATION_VERTICAL, 5),
        startButton("Start FMRI")
{
    set_size_request(400, -1);
    add(box);
    auto grid = new Gtk::Grid();
    Gtk::manage(grid);

    grid->set_row_spacing(2);
    grid->set_column_spacing(2);

    box.pack_start(*grid, true, true);
    box.pack_start(startButton);
    findExecutable();
    fmriChooser.set_hexpand(true);
    grid->attach(fmriChooser, 1, 0, 1, 1);
    grid->attach_next_to(*getManagedLabel("FMRI executable"), fmriChooser, Gtk::PositionType::POS_LEFT, 1, 1);
    grid->attach(modelChooser, 1, 1, 1, 1);
    grid->attach_next_to(*getManagedLabel("Model"), modelChooser, Gtk::PositionType::POS_LEFT, 1, 1);
    grid->attach(weightsChooser, 1, 2, 1, 1);
    grid->attach_next_to(*getManagedLabel("Weights"), weightsChooser, Gtk::PositionType::POS_LEFT, 1, 1);

    inputChooser.set_action(Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    grid->attach(inputChooser, 1, 3, 1, 1);
    grid->attach_next_to(*getManagedLabel("Input directory"), inputChooser, Gtk::PositionType::POS_LEFT, 1, 1);

    startButton.signal_clicked().connect(sigc::mem_fun(*this, &Launcher::start));
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
            (char*) executable.c_str(),
            (char*) "-n",
            (char*) network.c_str(),
            (char*) "-w",
            (char*) weights.c_str(),
    };

    std::transform(inputs.begin(), inputs.end(), std::back_inserter(argv),
            [](const auto& x) { return (char*) x.c_str(); });

    argv.push_back(nullptr);

    execv(executable.data(), argv.data());
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

    const char* env_path = std::getenv("PATH");
    if (env_path == nullptr || env_path[0] == '\0') {
        return;
    }

    std::unique_ptr<char[]> buffer(new char[std::strlen(env_path) + 1]);
    std::strcpy(buffer.get(), env_path);

    for (auto component = std::strtok(buffer.get(), ":"); component; component = std::strtok(nullptr, ":")) {
        if (executable_exists(component, "fmri")) {
            fmriChooser.set_filename(std::string(component) + "/fmri");
            return;
        }
    }
}

int main(int argc, char** argv)
{
    auto app = Gtk::Application::create(argc, argv);
    Launcher launcher;
    return app->run(launcher);
}
