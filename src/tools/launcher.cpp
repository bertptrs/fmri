#include <gtkmm.h>
#include <iostream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

class Launcher {
public:
    Launcher(int argc, char** argv);
    ~Launcher();
    void run();

private:
    Glib::RefPtr<Gtk::Application> app;
    Gtk::Window* window = nullptr;
    Gtk::FileChooserButton* fmriChooser = nullptr;
    Gtk::FileChooserButton* modelChooser = nullptr;
    Gtk::FileChooserButton* weightsChooser = nullptr;
    Gtk::FileChooserButton* inputChooser = nullptr;
    Gtk::Button* startButton = nullptr;

    void start();
    bool hasFile(const Gtk::FileChooserButton* fileChooser, const std::string& error);
    std::vector<std::string> getInputFiles();
};

Launcher::Launcher(int argc, char** argv)
        :
        app(Gtk::Application::create(argc, argv))
{
    auto builder = Gtk::Builder::create_from_file("../launcher/launcher.glade");
    builder->get_widget("window", window);
    builder->get_widget("fmriChooser", fmriChooser);
    builder->get_widget("modelChooser", modelChooser);
    builder->get_widget("weightsChooser", weightsChooser);
    builder->get_widget("inputChooser", inputChooser);
    builder->get_widget("startButton", startButton);

    if (access("fmri", F_OK | X_OK)==0) {
        // Set default path to FMRI.
        fmriChooser->set_file(Gio::File::create_for_path("fmri"));
    }
    startButton->signal_clicked().connect(sigc::mem_fun(*this, &Launcher::start));
}

void Launcher::run()
{
    if (window) {
        app->run(*window);
    }
    else {
        std::cerr << "Failed to load window" << std::endl;
        exit(2);
    }
}

Launcher::~Launcher()
{
    delete window;
}

void Launcher::start()
{
    if (!hasFile(fmriChooser, "Need executable") ||
            !hasFile(modelChooser, "Need model file") ||
            !hasFile(weightsChooser, "Need weights file") ||
            !hasFile(inputChooser, "Need input folder")) {
        return;
    }

    auto executable = fmriChooser->get_file()->get_path();
    auto network = modelChooser->get_file()->get_path();
    auto weights = weightsChooser->get_file()->get_path();
    auto inputs = getInputFiles();

    if (inputs.empty()) {
        Gtk::MessageDialog dialog(*window, "No inputs in folder!");
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
            [](const auto& x) -> auto { return (char*) x.c_str(); });

    argv.push_back(nullptr);

    execv(executable.data(), argv.data());
}

bool Launcher::hasFile(const Gtk::FileChooserButton* fileChooser, const std::string& error)
{
    if (!fileChooser->get_file()) {
        Gtk::MessageDialog dialog(*window, error);
        dialog.run();
        return false;
    }

    return true;
}

std::vector<std::string> Launcher::getInputFiles()
{
    using namespace boost::filesystem;

    std::vector<std::string> result;

    auto folder = path(inputChooser->get_file()->get_path());

    directory_iterator end_itr;
    for (directory_iterator itr(folder); itr!=end_itr; ++itr) {
        if (!is_regular(itr->status())) {
            continue;
        }

        result.push_back(itr->path().string());
    }

    return result;

}

int main(int argc, char** argv)
{
    Launcher launcher(argc, argv);
    launcher.run();

    // Should never be reached.
    return 0;
}
