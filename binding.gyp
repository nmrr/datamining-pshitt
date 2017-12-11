{
  "targets": [
    {
      "target_name": "addon",
      "sources": ["addon.cpp", "countrycode.cpp"],
      "cflags": ["-Wall", "-std=c++11", "-Ofast", "-pipe"],
      "cflags_cc": ["-fexceptions"],
      "ldflags" : [""],
      "libraries": [ "" ],
      "include_dirs" : ["<!(node -e \"require('nan')\")"],
    }
  ]
}

