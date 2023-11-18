add_library(asiosdk STATIC
	common/asio.h
	common/asio.cpp
	common/iasiodrv.h
	common/asiosys.h
	host/asiodrivers.h
	host/asiodrivers.cpp
	host/ASIOConvertSamples.h
	host/ASIOConvertSamples.cpp
	host/pc/asiolist.h
	host/pc/asiolist.cpp
)

target_include_directories(asiosdk
	PUBLIC
		common
		host
		host/pc
)
