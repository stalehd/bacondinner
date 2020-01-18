# Bring on the bacon!

Yes. The name is annoying. I write "beacon" all the time and keeps on saying
"bacon" whenever I want to say "beacon".

## Targets

```
targets/ee04_bacon
    app=apps/bacon
    bsp=@apache-mynewt-core/hw/bsp/telee02
    build_profile=debug
targets/ee04_boot
    app=@mcuboot/boot/mynewt
    bsp=@apache-mynewt-core/hw/bsp/telee02
```

## Building

```
newt update
newt build ee04_boot
newt build ee04_bacon
```

## Installing beacons

```
newt load ee04_boot
newt build ee04_bacon
newt create-image ee04_bacon
newt load ee04_bacon
```

The major and minor versions for the beacons are hard coded.
