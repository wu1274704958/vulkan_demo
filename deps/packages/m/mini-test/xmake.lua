package("mini-test")

    set_kind("library", {headeronly = true})
    set_homepage("https://github.com/wu1274704958/mini-test")
    set_description("cpp toolbox libraries.")
    set_license("MIT")

    set_urls("https://github.com/wu1274704958/mini-test.git")
    add_versions("0.2", "0d535bc469fd2a26a5e6a5d0167fc0620f428692")

    on_install(function (package)
        os.cp("include", package:installdir(""))
    end)

    on_test(function (package)
        
    end)