{
  "targets": [
    { 
      "target_name": ""
    },
  ],
  "conditions": [
    ['OS=="mac"', {
      "targets": [{
        "target_name": "fse",
        "sources": ["fsevents.cc"],
        "xcode_settings": {
          "OTHER_LDFLAGS": [
            "-framework CoreFoundation -framework CoreServices"
          ]
        },
        "include_dirs": [
          "<!(node -e \"require('nan')\")"
        ]
      }, {
        "target_name": "action_after_build",
        "type": "none",
        "dependencies": ["fse"],
        "copies": [{
          "files": ["<(PRODUCT_DIR)/fse.node"],
          "destination": "./build/Debug/"
        }]
      }]
    }]
  ]
}
