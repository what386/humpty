set_project("humpty")
set_version("0.1.0")

set_languages("cxx23")
add_rules("mode.debug", "mode.release")

target("humpty_lib")
    set_kind("static")
    add_files("src/**.cpp")
    remove_files("src/main.cpp")
    add_includedirs("src", {public = true})

target("humpty")
    set_kind("binary")
    add_files("src/main.cpp")
    add_deps("humpty_lib")
    add_includedirs("src")

target("humpty_tests")
    set_kind("binary")
    add_files("tests/**.cpp")
    add_deps("humpty_lib")
    add_includedirs("src", "tests")
