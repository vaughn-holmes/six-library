from conans import ConanFile, CMake, tools

class SixConan(ConanFile):
    name = "six-library"
    url = "https://github.com/ngageoint/six-library"
    description = "library for reading and writing the Sensor Independent XML (SIX) format"
    settings = "os", "compiler", "build_type", "arch"
    requires = ("coda-oss/CMake_update_win_ede7427059e489bc@user/testing",
                "nitro/cmake-integration_ba72c0500fd4824a@user/testing")
    options = {"shared": [True, False],
               }
    default_options = {"shared": False,
                       }
    exports_sources = ("CMakeLists.txt",
                       "LICENSE",
                       "README.md",
                       "cmake/*",
                       "six/*",
                       )
    generators = "cmake_paths"
    license = "GNU LESSER GENERAL PUBLIC LICENSE Version 3"

    def set_version(self):
        git = tools.Git(folder=self.recipe_folder)
        self.version = "%s_%s" % (git.get_branch(), git.get_revision()[:16])

    def _configure_cmake(self):
        cmake = CMake(self)
        if self.settings.compiler == "Visual Studio":
            cmake.definitions["CMAKE_BUILD_TYPE"] = self.settings.build_type
        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()

    def package_id(self):
        # Make any change in our dependencies' version require a new binary
        self.info.requires.full_version_mode()

    def package_info(self):
        self.cpp_info.builddirs = ["lib/cmake"]
