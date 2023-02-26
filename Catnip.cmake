
catnip_package(maxmod DEFAULT all)

catnip_add_preset(ds7
	TOOLSET    NDS
	PROCESSOR  armv4t
	BUILD_TYPE MinSizeRel
	CACHE      DKP_NDS_PLATFORM_LIBRARY=calico
)

catnip_add_preset(ds9
	TOOLSET    NDS
	PROCESSOR  armv5te
	BUILD_TYPE Release
	CACHE      DKP_NDS_PLATFORM_LIBRARY=calico
)

catnip_add_preset(ds9_hybrid
	TOOLSET    NDS
	PROCESSOR  armv5te
	BUILD_TYPE Release
	CACHE      NDS_ROOT=${DEVKITPRO}/libnds-calico
)
