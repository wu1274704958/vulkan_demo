package("mini-test")

    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/wu1274704958/mini-test")
    set_description("cpp toolbox libraries.")
    set_license("MIT")

    set_urls("https://github.com/wu1274704958/mini-test.git")
    add_versions("0.21", "21a1d5ca2e15b29a62e0d0337f1da00883084a70")

    on_install(function (package)
        os.cp("include", package:installdir(""))
    end)

    on_test(function (package)
        
    end)