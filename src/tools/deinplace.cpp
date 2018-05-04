#include <getopt.h>
#include <fstream>
#include <memory>
#include <fcntl.h>
#include <iostream>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <caffe/proto/caffe.pb.h>

static struct
{
    bool verbose = false;
    std::string_view output_filename;
    std::string_view input_filename;
} options;

template<typename... Args>
void verbose_print(Args &&... args)
{
    if (!options.verbose) return;

    (std::cerr << ... << args) << '\n';
}

void show_help(std::string_view name, int exit_code)
{
    std::cerr << "Usage: " << name << "[OPTIONS] [FILENAME]\n"
            "\n"
            "Valid options:\n"
            "-h\tshow this message\n"
            "-v\tshow debug messages\n"
            "-o OUTPUT\t write output to OUTPUT\n"
            "FILENAME\t read from FILENAME instead of stdin\n" << std::endl;

    exit(exit_code);
}

int get_file_descriptor(std::string_view file_name, int mode)
{
    int fd;
    if ((mode & O_CREAT) || (mode & O_TMPFILE)) {
        fd = open(file_name.data(), mode, 0666);
    } else {
        fd = open(file_name.data(), mode);
    }
    if (fd < 0) {
        perror("get_file_descriptor");
        exit(1);
    }

    return fd;
}

void read_options(int argc, char **argv)
{
    for (char c; (c = static_cast<char>(getopt(argc, argv, "hvo:"))) != -1;) {
        switch (c) {
            case 'v':
                options.verbose = true;
                break;
            case 'h':
                show_help(argv[0], 0);
                break;
            case 'o':
                options.output_filename = optarg;
                break;
            case '?':
            default:
                show_help(argv[0], 1);
                break;
        }
    }

    if (argc > optind) {
        options.input_filename = argv[optind];
    }
}

void read_network(caffe::NetParameter &network)
{
    using namespace google::protobuf::io;
    std::unique_ptr<ZeroCopyInputStream> input;
    if (!options.input_filename.empty()) {
        verbose_print("Reading network from", options.input_filename);

        auto fd = get_file_descriptor(options.input_filename, O_RDONLY);
        FileInputStream *i = new FileInputStream(fd);
        i->SetCloseOnDelete(true);
        input.reset(i);
    } else {
        verbose_print("Reading network from stdin");
        input = std::make_unique<IstreamInputStream>(&std::cin);
    }

    auto result = google::protobuf::TextFormat::Parse(input.get(), &network);
    if (!result) {
        std::cerr << "Error reading network file!" << std::endl;
        exit(2);
    }
}

template<typename LayerType>
void patch_layers(google::protobuf::RepeatedPtrField<LayerType> *layers)
{
    std::map<std::string, std::string> outputs;

    for (LayerType &layer : *layers) {

        for (int i = 0; i < layer.bottom_size(); ++i) {
            if (auto it = outputs.find(layer.bottom(i)); it != outputs.end()) {
                verbose_print(layer.name(), " reads from in-place ", layer.bottom(i), ", rewriting.");
                *layer.mutable_bottom(i) = it->second;
            }
        }

        for (int i = 0; i < layer.top_size(); ++i) {
            if (auto it = outputs.find(layer.top(i)); it != outputs.end()) {
                verbose_print(layer.name(), " works in-place rewriting.");
                it->second = layer.name();
                *layer.mutable_top(i) = layer.name();
            } else {
                outputs[layer.name()] = layer.name();
            }
        }
    }
}

void patch_network(caffe::NetParameter &network)
{
    patch_layers(network.mutable_layer());
    patch_layers(network.mutable_layers());
}

void write_network(caffe::NetParameter &network)
{
    using namespace google::protobuf::io;
    std::unique_ptr<ZeroCopyOutputStream> output;
    if (!options.output_filename.empty()) {
        verbose_print("Writing network to ", options.output_filename);

        auto fd = get_file_descriptor(options.output_filename, O_WRONLY | O_CREAT | O_TRUNC);
        FileOutputStream *i = new FileOutputStream(fd);
        i->SetCloseOnDelete(true);
        output.reset(i);
    } else {
        verbose_print("Writing network to stdout");
        output = std::make_unique<OstreamOutputStream>(&std::cout);
    }

    google::protobuf::TextFormat::Print(network, output.get());
}

int main(int argc, char **argv)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    read_options(argc, argv);
    caffe::NetParameter network;
    read_network(network);
    patch_network(network);
    write_network(network);

    // Deallocate all global protobuf objects.
    google::protobuf::ShutdownProtobufLibrary();
}
