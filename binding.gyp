{
  "targets": [
    {
      "target_name": "win_browser_url",
			'cflags!': [ '-fno-exceptions' ],
  		'cflags_cc!': [ '-fno-exceptions' ],
  		'conditions': [
				["OS=='win'", {
					"sources": ["cpp/windows_browser_url.cc"],
					"include_dirs": ["<!(node -p \"require('node-addon-api').include_dir\")"],
					"defines": [
						"_HAS_EXCEPTIONS=1"
					],
					"msvs_settings": {
						"VCCLCompilerTool": {
							"ExceptionHandling": 1
						},
					},
				}, { # "OS!='win'"
					"sources": ["cpp/empty.cc"],
				}],
				["OS=='mac'", {
					'xcode_settings': {
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
						'CLANG_CXX_LIBRARY': 'libc++',
						'MACOSX_DEPLOYMENT_TARGET': '10.7',
					},
				}],
  		],
		}
  ]
}
