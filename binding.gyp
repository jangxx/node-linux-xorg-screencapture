{
    "targets": [
        {
            "target_name": "xscreencap",
            "sources": [
                "src/getframeasyncworker.cpp",
                "src/xscreencap.cpp"
            ],
            "include_dirs": [
                "<!@(node -p \"require('node-addon-api').include\")"
            ],
            "dependencies": [
                "<!(node -p \"require('node-addon-api').gyp\")"
            ],
            "defines": [
                "NAPI_DISABLE_CPP_EXCEPTIONS"
            ],
            "libraries": [
                "<!@(pkg-config --cflags --libs x11)",
                "<!@(pkg-config --cflags --libs xrandr)"
            ],
            "cflags": [
                "-pthread"
            ]
        }
    ]
}