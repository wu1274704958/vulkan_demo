package("mini-test")

    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/wu1274704958/mini-test")
    set_description("cpp toolbox libraries.")
    set_license("MIT")

    set_urls("https://github.com/wu1274704958/mini-test.git")
    add_versions("0.1", "e4c3282414f519ce1c58bd20e5dbe45bf65f5f62")

    on_install(function (package)
        os.cp("include", package:installdir(""))
    end)

    on_test(function (package)
        
    end)