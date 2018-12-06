{
  "targets": [
    {
      "target_name": "<(module_name)",
      "sources": [ "setFolderReadOnly.cc" ],
	  "defines": [
	  "NAPI_DISABLE_CPP_EXCEPTIONS",
      "NAPI_VERSION=<(napi_build_version)",
	  ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")"
      ],
    },
	{
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
