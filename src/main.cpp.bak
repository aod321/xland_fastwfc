//
// Created by yinzi on 2022/11/21.
//
#include "xml_wfc.hpp"
#include <filesystem>
#include <tuple>
#include "iostream"
#include <time.h>


int main() {
    auto random_seed = XMLWFC::get_random_seed();
    XMLWFC::XLandWFC wfc("samples.xml");
    for(int i = 0; i < 10; i++) {
        auto [result, img] = wfc.run(true);
//    auto [result, img] = XMLWFC::XLandWFC::run("samples.xml",random_seed,true);
        auto wave = wfc.build_wave_from_ids(result);
        if (img.has_value()) {
            std::cout<<"generate success"<<std::endl;
            auto name = "./results/" + std::to_string(i) + ".png";
            write_image_png(name, *img);
        }
    }

//    XMLWFC::XLandWFC wfc2("samples.xml");
//    auto [result2, img2] = XMLWFC::XLandWFC::run("samples.xml",random_seed,true);
//    auto [result2, img2] =  XMLWFC::XLandWFC::mutate(*wave,81,random_seed,"samples.xml", 1, true);
//    auto [result2, img2] =  wfc2.mutate(*wave,81,1, true);
//    auto [result2, img2] =  wfc2.run(true);
//    if (img2.has_value()) {
//        std::cout<<"mutate success"<<std::endl;
//        write_image_png("results/test_1_mutate.png", *img2);
//    }

    return 0;
}
