# To build :
#   node-gyp configure
#   node-gyp build
{
  "targets": [
    {
      "target_name": "shm",
      "sources": [ "src/SHM.cc", "src/SHMobject.cc", "src/shmop.c" ]
    }
#   ,{
#      "target_name": "shm_cli",
#	  'type': 'executable',
#      "defines": [
#        'USE_AS_CONSOLE'
#      ],
#      "sources": [ "src/shmop.c", "src/shmop_cli.c" ]
#    }
#   ,{
#      "target_name": "shm_test",
#	  'type': 'executable',
#      "defines": [
#        'TEST_JS_SHMOP'
#      ],
#      "sources": [ "src/shmop.c", "src/shmop_cli.c" ]
#    }
  ]
}