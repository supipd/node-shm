# To build :
#   node-gyp configure
#   node-gyp build
{
  "targets": [
    {
      "target_name": "shm",
      "sources": [ "SHM.cc", "SHMobject.cc", "shmop.c" ]
    }
   ,{
      "target_name": "shm_cli",
	  'type': 'executable',
      "defines": [
        'USE_AS_CONSOLE'
      ],
      "sources": [ "shmop.c", "shmop_cli.c" ]
    }
   ,{
      "target_name": "shm_test",
	  'type': 'executable',
      "defines": [
        'TEST_JS_SHMOP'
      ],
      "sources": [ "shmop.c", "shmop_cli.c" ]
    }
  ]
}