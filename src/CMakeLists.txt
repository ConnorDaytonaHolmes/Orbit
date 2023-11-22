cmake_minimum_required (VERSION 3.13)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(driver
	"driver/asio/asioconfig.cpp"
	"driver/asio/asioconfig.h"
	"driver/asio/focusrite18i20.h"
	"driver/wasapi/wasapiconfig.cpp"
	"driver/wasapi/wasapiconfig.h"
	"driver/asio/asio_util.h"
	"driver/asio/asio_util.cpp"
	"driver/devicestate.h"
)
target_include_directories(driver PUBLIC
	"driver"
	"../${ASIO_sdk}/"
	"../${ASIO_sdk}/common"
	"../${ASIO_sdk}/host"
	"../${ASIO_sdk}/host/pc"
)
target_link_libraries(driver PUBLIC asiosdk)

add_library(engine
	"engine/audiobuffer.cpp"
	"engine/audiobuffer.h"
	"engine/iaudiochannel.cpp"
	"engine/iaudiochannel.h"
	"engine/iaudiooutput.cpp"
	"engine/iaudiooutput.h"
	"engine/iaudioinput.cpp"
	"engine/iaudioinput.h"
	"engine/masterbuffer.cpp"
	"engine/masterbuffer.h"
	"engine/mixer.cpp"
	"engine/mixer.h"
	"engine/route.cpp"
	"engine/route.h"
	"engine/sampletype.cpp"
	"engine/sampletype.h"
	"engine/engine.h"
	"engine/engine.cpp"
	"engine/audiosettings.h"
	"engine/igenerator.h"
	"engine/igenerator.cpp"
)
target_include_directories(engine PUBLIC "engine")

add_library(wav
	"wav/parsewav.cpp"
	"wav/parsewav.h"
	"wav/wavheader.h"
)
target_include_directories(wav PUBLIC "wav")

add_library(wavetable
	"wavetable/saw.h"
	"wavetable/sine.h"
	"wavetable/square.h"
	"wavetable/wavetable.cpp"
	"wavetable/wavetable.h"
	"wavetable/wavetablecollection.cpp"
	"wavetable/wavetablecollection.h"
)
target_include_directories(wavetable PUBLIC "wavetable")

add_library(ui
	"ui/wavplayer.h"
	"ui/wavplayer.cpp"
)
target_include_directories(ui PUBLIC "ui")
target_link_libraries(ui PUBLIC
	engine
	${imguiLib}
	${WalnutLib}
	${glfwLib}
)