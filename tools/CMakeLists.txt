add_executable(asset_baker
    asset_baker.c
    cfg_parse.c
    ../src/xcr.c
    ../src/scn.c
    ../src/image.c
    ../src/ani.c
    ../src/file_io.c
)

recon_target_defaults(asset_baker)

target_include_directories(asset_baker PRIVATE ../src)
