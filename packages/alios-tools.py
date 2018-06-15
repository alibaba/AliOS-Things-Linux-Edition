#!/usr/bin/env python

import os

def add_gitkeep():
    for path,dirs,fs in os.walk('./'):
        for f in dirs:
            print(f)
            if not os.listdir(os.path.join(path,f)):
                print(os.path.join(path,f))
                os.mknod(os.path.join(path,f) + "/.gitkeep")

if __name__ == "__main__":
    add_gitkeep()
