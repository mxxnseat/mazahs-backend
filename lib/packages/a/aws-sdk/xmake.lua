package("aws-sdk")
    set_homepage("https://github.com/aws/aws-sdk-cpp/tree/main")
    set_description("A c++ AWS SDK")

    add_urls("git@github.com:aws/aws-sdk-cpp.git")

    add_deps("cmake")

    on_install("windows", "linux", "macosx", "iphoneos", "mingw", "android", function (package)
        local configs = {
            "-DCMAKE_BUILD_TYPE=Debug",
            "-DCMAKE_INSTALL_PREFIX=" .. package:installdir(),
            "-DBUILD_ONLY=s3"
        }

        import("package.tools.cmake").install(package, configs)
    end)
