
catnip_package(maxmod DEFAULT all)

catnip_add_preset(ds7
	TOOLSET    NDS
	PROCESSOR  armv4t
	BUILD_TYPE MinSizeRel
)

catnip_add_preset(ds9
	TOOLSET    NDS
	PROCESSOR  armv5te
	BUILD_TYPE Release
)
