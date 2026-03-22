add_executable(asset_baker
    asset_baker.c
    stb_image_write.c
    cfg_parse.c
    ../src/xcr.c
    ../src/image.c
    ../src/ani.c
    ../src/file_io.c
    ../src/stb_image.c
)

recon_target_defaults(asset_baker)

target_include_directories(asset_baker PRIVATE ../src)
