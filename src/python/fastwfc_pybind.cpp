#include <pybind11/pybind11.h>
#include "wfc.hpp"
#include "wave.hpp"
#include "color.hpp"
//#include "propagator.hpp"
#include "xml_wfc.hpp"
#include "tiling_wfc.hpp"
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <utility>
#include <filesystem>
#include <system_error>
template<typename Tuple>
using make_tuple_index_sequence = std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>;



#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

int add(int i, int j) {
    return i + j;
}

int subtract(int i, int j) {
    return i - j;
}

namespace py = pybind11;

namespace {

std::string default_config_path() {
    py::gil_scoped_acquire acquire;
    auto resources = py::module_::import("fastwfc_resources");
    auto path_obj = resources.attr("get_samples_xml_path")();
    return path_obj.cast<std::string>();
}

std::string resolve_config_path(const std::string &config_path) {
    namespace fs = std::filesystem;
    auto default_path = default_config_path();
    if (config_path.empty()) {
        return default_path;
    }

    fs::path candidate(config_path);
    std::error_code ec;
    if (fs::exists(candidate, ec)) {
        return candidate.string();
    }

    fs::path package_root = fs::path(default_path).parent_path();
    fs::path relative_candidate = package_root / candidate;
    if (fs::exists(relative_candidate, ec)) {
        return relative_candidate.string();
    }

    return default_path;
}

}  // namespace


class PyWave: public Wave
{
    public:
    using Wave::Wave;
    PyWave(unsigned height, unsigned width,
            const std::vector<double> &patterns_frequencies):
            Wave(height, width, patterns_frequencies){};

    std::vector<unsigned > get_data() {
        std::vector<unsigned> wave_ids;
        for(int i = 0; i < size; i++){
            for(int k = 0; k < nb_patterns; k++){
                if(get(i, k)){
                    wave_ids.push_back(k);
                }
            }
        }
        return wave_ids;
    }

    void set_data(std::vector<unsigned> wave_ids){
        for(int i = 0; i < size; i++){
            for(int k = 0; k < nb_patterns; k++){
                set(i, k, false);
            }
            set(i, wave_ids[i], true);
        }
    }

};

class PyXLandWFC: public XMLWFC::XLandWFC{
public:
    PyXLandWFC (const std::string& config_path):
        XMLWFC::XLandWFC(resolve_config_path(config_path)){};

    auto generate(bool out_img=false){
         auto [output_patterns, output_colors]  = XMLWFC::XLandWFC::run(out_img);
         if(output_colors.has_value() && out_img){
            auto output_img = output_colors -> data;
            vector<uint8_t> output_img_vec;
            for(auto & i : output_img){
                output_img_vec.push_back(i.r);
                output_img_vec.push_back(i.g);
                output_img_vec.push_back(i.b);
            }
             // convert output_img to 3d numpy array, with shape (48*9,48*9,3)
             unsigned const height = output_colors -> height;
             unsigned const width = output_colors -> width;
             auto result = py::array_t<uint8_t>(std::vector<unsigned>{height, width, 3});
             auto buf = result.request();
             auto ptr = static_cast<uint8_t *>(buf.ptr);
             std::copy(output_img_vec.begin(), output_img_vec.end(), ptr);
             return py::make_tuple(output_patterns, result);
         } else
         {
             return py::make_tuple(output_patterns, py::none());
         }
    }

    static auto build_a_open_area_wave_static(const std::string _config_path){
        auto wave = XMLWFC::XLandWFC::build_a_open_area_wave(resolve_config_path(_config_path));
        auto new_wave = new PyWave(wave->height, wave->width, wave->patterns_frequencies);
        new_wave->data = wave->data;
        delete wave;
        return new_wave;
    }

     auto build_a_open_area_wave(){
        auto wave = XMLWFC::XLandWFC::build_a_open_area_wave();
        auto new_wave = new PyWave(wave->height, wave->width, wave->patterns_frequencies);
        new_wave->data = wave->data;
        delete wave;
        return new_wave;
    }

    auto wave_from_id(vector<pair<unsigned, unsigned>> &tiles){
        auto wave =  XMLWFC::XLandWFC::build_wave_from_ids(tiles);
        auto new_wave = new PyWave(wave->height, wave->width, wave->patterns_frequencies);
        new_wave->data = wave->data;
        delete wave;
        return new_wave;
    }

    static auto wave_from_id_static(vector<pair<unsigned, unsigned>> &tiles, const std::string _config_path){
        auto wave =  XMLWFC::XLandWFC::build_wave_from_ids(tiles, resolve_config_path(_config_path));
        auto new_wave = new PyWave(wave->height, wave->width, wave->patterns_frequencies);
        new_wave->data = wave->data;
        delete wave;
        return new_wave;
    }

    auto get_ids_from_wave(PyWave &wave){
        auto ids = XMLWFC::XLandWFC::get_ids_from_wave(wave);
        return ids;
    }

    static auto get_ids_from_wave_static(PyWave &wave, const std::string _config_path){
        auto ids = XMLWFC::XLandWFC::get_ids_from_wave(wave, resolve_config_path(_config_path));
        return ids;
    }

    auto mutate(PyWave wave, double new_weight, int iter_count=1, bool out_img=false){
        auto [output_patterns, output_colors]  =
                XMLWFC::XLandWFC::mutate(wave, new_weight, iter_count, out_img);
        if(output_colors.has_value() && out_img){
            auto output_img = output_colors -> data;
            vector<uint8_t> output_img_vec;
            for(auto & i : output_img){
                output_img_vec.push_back(i.r);
                output_img_vec.push_back(i.g);
                output_img_vec.push_back(i.b);
            }
            // convert output_img to 3d numpy array, with shape (48*9,48*9,3)
            unsigned const height = output_colors -> height;
            unsigned const width = output_colors -> width;
            auto result = py::array_t<uint8_t>(std::vector<unsigned>{height, width, 3});
            auto buf = result.request();
            auto ptr = static_cast<uint8_t *>(buf.ptr);
            std::copy(output_img_vec.begin(), output_img_vec.end(), ptr);
            return py::make_tuple(output_patterns, result);
        } else
        {
            return py::make_tuple(output_patterns, py::none());
        }
    }

    static auto generate_static(const std::string _config_path, const unsigned _seed, bool out_img=false){
        auto [output_patterns, output_colors]  = XMLWFC::XLandWFC::run(resolve_config_path(_config_path), _seed, out_img);
        if(output_colors.has_value() && out_img){
            auto output_img = output_colors -> data;
            vector<uint8_t> output_img_vec;
            for(auto & i : output_img){
                output_img_vec.push_back(i.r);
                output_img_vec.push_back(i.g);
                output_img_vec.push_back(i.b);
            }
            // convert output_img to 3d numpy array, with shape (48*9,48*9,3)
            unsigned const height = output_colors -> height;
            unsigned const width = output_colors -> width;
            auto result = py::array_t<uint8_t>(std::vector<unsigned>{height, width, 3});
            auto buf = result.request();
            auto ptr = static_cast<uint8_t *>(buf.ptr);
            std::copy(output_img_vec.begin(), output_img_vec.end(), ptr);
            return py::make_tuple(output_patterns, result);
        } else
        {
            return py::make_tuple(output_patterns, py::none());
        }
    }

    static auto mutate_static(PyWave wave, double new_weight, unsigned _seed,const std::string _config_path,int iter_count=1, bool out_img=false){
        auto [output_patterns, output_colors]  =
                XMLWFC::XLandWFC::mutate(wave, new_weight,_seed,resolve_config_path(_config_path),iter_count, out_img);
        if(output_colors.has_value() && out_img){
            auto output_img = output_colors -> data;
            vector<uint8_t> output_img_vec;
            for(auto & i : output_img){
                output_img_vec.push_back(i.r);
                output_img_vec.push_back(i.g);
                output_img_vec.push_back(i.b);
            }
            // convert output_img to 3d numpy array, with shape (48*9,48*9,3)
            unsigned const height = output_colors -> height;
            unsigned const width = output_colors -> width;
            auto result = py::array_t<uint8_t>(std::vector<unsigned>{height, width, 3});
            auto buf = result.request();
            auto ptr = static_cast<uint8_t *>(buf.ptr);
            std::copy(output_img_vec.begin(), output_img_vec.end(), ptr);
            return py::make_tuple(output_patterns, result);
        } else
        {
            return py::make_tuple(output_patterns, py::none());
        }
    }
};

PYBIND11_MODULE(fastwfc, m) {
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: fastwfc

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

    m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");
    m.def("subtract", &subtract, R"pbdoc(
        Subtract two numbers
    )pbdoc");

    py::class_<Color>(m, "Color")
        .def(py::init<>())
        // .def(py::init<uint8_t, uint8_t, uint8_t>())
        .def_readwrite("r", &Color::r)
        .def_readwrite("g", &Color::g)
        .def_readwrite("b", &Color::b)
        .def("__eq__", &Color::operator==)
        .def("__ne__", &Color::operator!=)
        .def("__hash__", [](const Color &c) { return std::hash<Color>()(c); });

    py::class_<PyWave>(m, "Wave")
    .def(py::init<unsigned,unsigned,const std::vector<double>>())
    .def("get_data", &PyWave::get_data,  R"pbdoc(
        Get Wave Data
    )pbdoc")
    .def("set_data", &PyWave::set_data,  R"pbdoc(
        Set Wave Data
    )pbdoc");
    py::class_<PyXLandWFC>(m, "XLandWFC")
    .def(py::init([](py::object config_path){
        if(config_path.is_none()){
            return new PyXLandWFC(default_config_path());
        }
        return new PyXLandWFC(config_path.cast<std::string>());
    }), py::arg("config_path") = py::none())
    .def("generate", &PyXLandWFC::generate,
         py::arg("out_img")=false,
         R"pbdoc(
        Generate a Map)pbdoc")
    .def("build_a_open_area_wave", &PyXLandWFC::build_a_open_area_wave,
            R"pbdoc(
            Build a open area wave)pbdoc")
    .def("wave_from_id", &PyXLandWFC::wave_from_id,
            py::arg("tile_ids"),
            R"pbdoc(
            Build a wave from ids)pbdoc")
    .def("get_ids_from_wave", &PyXLandWFC::get_ids_from_wave,
         py::arg("wave"),
         R"pbdoc(
            Get ids from wave)pbdoc")
    .def("mutate", &PyXLandWFC::mutate,
         py::arg("base_wave"),
         py::arg("new_weight"),
         py::arg("iter_count"),
         py::arg("out_img"),
         R"pbdoc(Mutate a wave)pbdoc")
    .def_static("mutate_static", &PyXLandWFC::mutate_static,
            py::arg("base_wave"),
            py::arg("new_weight"),
            py::arg("seed"),
            py::arg("config_path") = "",
            py::arg("iter_count") = 1,
            py::arg("out_img") = false,
            R"pbdoc(Mutate a wave static)pbdoc")
    .def_static("generate_static", &PyXLandWFC::generate_static,
            py::arg("config_path") = "",
            py::arg("seed"),
            py::arg("out_img") = false,
            R"pbdoc(Generate a Map static)pbdoc")
    .def_static("build_a_open_area_wave_static", &PyXLandWFC::build_a_open_area_wave_static,
            py::arg("config_path") = "",
            R"pbdoc(
            Build a open area wave static)pbdoc")
    .def_static("wave_from_id_static", &PyXLandWFC::wave_from_id_static,
            py::arg("tile_ids"),
            py::arg("config_path") = "",
            R"pbdoc(
            Build a wave from ids static)pbdoc")
    .def_static("get_ids_from_wave_static", &PyXLandWFC::get_ids_from_wave_static,
            py::arg("wave"),
            py::arg("config_path") = "",
            R"pbdoc(
            Get ids from wave static)pbdoc");




#ifdef VERSION_INFO
    m.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
