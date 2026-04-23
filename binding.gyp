{
    'dependencies': 
    {
        'node-addon-api': '*',
    },
    'target_defaults': 
    {
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "include_dirs": 
        [
            ## This is theoretically required but causes the build to fail: 
            ##"<!@(node -p \"require('node-addon-api').include\")",
            ## This also does not work:
            ## "/usr/local/lib/nodejs/$(NODEJS_VERSION)-$(NODEJS_VERSION)/lib/node_modules/node-addon-api",
            ## This does work but must be manually configured here:
            ## "/usr/local/lib/node-v12.14.1-linux-x64/lib/node_modules/node-addon-api",
            '<!@(printf "%s" "$NODE_ADDON_API_INCLUDE")'
        ],
        'defines': 
        [
            'NAPI_DISABLE_CPP_EXCEPTIONS',
        ],
        'conditions': 
        [
            ['OS=="linux"',
                {
                    'cflags': [
                        '-std=c++14',
                        '-fno-exceptions',
                        '-Wno-deprecated-declarations',
                        '-fPIC',
                    ],
                    'libraries': 
                    [
                        '-lcsound64 -lecl -lgc -lpthread',
                    ],
                    'include_dirs': 
                    [
                        '/usr/local/include/csound/',
                        '/usr/include/csound/',
                    ],
                }
            ],
            ['OS=="mac"',
                {
                    'cflags': 
                    [
                        '-target x86_64-apple-darwin-macho',
                        '-mcpu=x86-64',
                        '-std=c++14',
                        '-fno-exceptions',
                        '-Wno-deprecated-declarations',
                        '-Wno-register',
                        '-fPIC',
                    ],
                    'cflags_cc': 
                    [
                        '-target x86_64-apple-darwin-macho',
                        '-mcpu=x86-64',
                        '-std=c++14',
                        '-fno-exceptions',
                        '-Wno-deprecated-declarations',
                        '-Wno-register',
                        '-fPIC',
                    ],
                    'link_settings': 
                    {
                        'libraries': 
                        [
                          '/Library/Frameworks/CsoundLib64.framework/Versions/7.0/CsoundLib64',
                          '-lpthread',
                        ],
                    },
                    'include_dirs': 
                    [
                        '/Library/Frameworks/CsoundLib64.framework/Versions/7.0/Headers/',
                        '/opt/homebrew/Cellar/ecl/21.2.1_1/include/',
                        '/opt/homebrew/Cellar/gmp/6.2.1_1/include/',
                        '/opt/homebrew/Cellar/bdw-gc/8.0.6/include/',
                        '/opt/homebrew/lib/node_modules/node-addon-api/',
                        '/Users/michaelgogins/csound-ac/CsoundAC/',
                    ],
                }
            ],
        ],
    },
    'targets': 
    [
        {
            'target_name': 'csound',
            'sources': 
            [
               'jscsound.cpp',
            ],
        },
    ]    
}
