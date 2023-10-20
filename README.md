# mrbtris on Wii - Proof of Concept

## WIP

This is an early-stage Work In Progress project.
It is currently being actively developed (as at 20 Oct 2023).
A lot of things will improve in the near future.

## Build

The build is currently very specific to my machine in terms of paths.
It will be addressed quite soon.

At the moment, you need to check out my fork of mruby which contains the branch with a build config file for Wii. Run this to clone and check out the branch:
```
git clone https://github.com/yujiyokoo/mruby.git -b add-wii-build-config
```

For the moment, you have to change the paths in Makefile from `/media/psf/Home/Repos/parallels-ubuntu/mruby` to wherever you have my fork of mruby checked out.

Then, you have to build mruby for Wii. In mruby project root, run:
```
make MRUBY_CONFIG=nintendo_wii
```

Also, the mruby compiler has to be invoked manually at the moment.
This will be addressed soon, but for now, in this project directory, do:

```
cd source/ && mrbc -g -Bprogram program.rb && cd .. && make
```

You should end up with a `.dol` file which you can run on Dolphin or Homebrew Channel if you have it installed on your Wii.
