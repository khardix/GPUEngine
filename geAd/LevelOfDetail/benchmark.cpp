#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include <AssimpModelLoader.h>
#include <LoDGenerator.h>

static const auto USAGE = R"usage(Usage:
    LoDGeneratorBenchmark <MODEL> <FRAC>

    MODEL -- Path to the model to simplify.
    FRAC  -- Fraction (0.0 to 0.9999...) of original element count.
)usage";

class usage_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

int main(int argc, char *argv[]) try {
    using std::chrono::duration_cast;
    using std::chrono::milliseconds;

    if (argc < 3) {
        throw usage_error("Not enough parameters!");
    }

    auto scene
        = std::unique_ptr<ge::sg::Scene>(AssimpModelLoader::loadScene(argv[1]));
    if (scene == nullptr) {
        throw usage_error("Invalid or unsupported model data!");
    }

    auto fraction = lod::ElementFraction{std::stod(argv[2])};

    auto &mesh = scene->models.at(0)->meshes.at(0);
    auto  graph = lod::Mesh(*mesh);

    const auto start = std::chrono::steady_clock::now();
    lod::simplify(graph, fraction);
    const auto stop = std::chrono::steady_clock::now();

    auto measurement = duration_cast<milliseconds>(stop - start);
    std::cout << measurement.count() << '\n';

    return 0;
}
catch (const usage_error &err) {
    std::cerr << err.what() << '\n';
    std::cerr << USAGE;
    return EXIT_FAILURE;
}
catch (const std::runtime_error &err) {
    std::cerr << err.what() << '\n';
    return EXIT_FAILURE;
}
