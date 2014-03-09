solution "server"
    kind        "ConsoleApp"
    language    "C"
    objdir      "build"
    targetdir   "bin"
    libdirs     { "src", "deps" }
    buildoptions { "-std=c99", "-pipe", "-Wno-parentheses" }
    configurations  { "Debug", "Release" }

    configuration "Debug"
        flags { "Symbols", "OptimizeSpeed", "ExtraWarnings" }
        defines { "_DEBUG" }

    configuration "Release"
        flags { "OptimizeSpeed" }

    project "server"
        includedirs { "src/", "deps/", "deps/libeio/" }
        files       { "src/**.c", "deps/**.c" }
        excludes    { "src/test*" }
        links       { "ev", "lua" }
