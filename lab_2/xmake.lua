add_rules("mode.debug", "mode.release")
set_languages("c++23")

target("1")
set_kind("binary")
add_files("lab_2_1.cc")

target("2")
set_kind("binary")
add_files("lab_2_2.cc")
