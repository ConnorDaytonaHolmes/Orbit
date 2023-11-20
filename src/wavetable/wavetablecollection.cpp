#include "wavetablecollection.h"
#include "sine.h"
#include "saw.h"
#include "square.h"

namespace wavetable {

	const unsigned int WAVETABLE_SIZE_BYTES = WAVETABLE_SIZE * sizeof(float);

	int WavetableCollection::load_folder(fsys::path const& root) {
		if (!fsys::exists(root) || !fsys::is_directory(root)) {
			return -1;
		}
		int num_loaded = 0;
		int i = 0;
		for (auto const& entry : fsys::recursive_directory_iterator(root)) {
			bool is_regular_file = fsys::is_regular_file(entry);
			std::string entry_ext = entry.path().extension().string();
			bool is_wt = entry_ext == WAVETABLE_EXTENSION;
			if (is_regular_file && is_wt) {
				int result = load_wavetable(entry.path());
				if (result != -1)
					num_loaded++;
			}
		}
		return num_loaded;
	}

	WavetableCollection::~WavetableCollection() {
		for (Wavetable* wt : wavetables) {
			delete wt;
		}		
		wavetables.clear();
	}

	int WavetableCollection::load_wavetable(fsys::path const& path) {		
		Wavetable* wt = new Wavetable;

		// Open file
		std::ifstream fs(path, std::ios::binary);
		if (!fs.is_open() || fs.fail()) {
			printf("Failed to open file '%s'\n", path.string().c_str());
			fs.close();
			return -1;
		}

		// Check file size
		fs.seekg(0, fs.end);
		auto length_of_file = fs.tellg();
		fs.seekg(0, fs.beg);
		if (length_of_file < WAVETABLE_SIZE_BYTES || length_of_file > WAVETABLE_SIZE_BYTES) {
			printf(
				"Size of file '%s' is %d bytes, expected %d bytes.\n"
				"Data will either be zero-initialized or not read.",
				path.string().c_str(),
				length_of_file,
				WAVETABLE_SIZE_BYTES
			);
		}

		char* file_data = new char[WAVETABLE_SIZE_BYTES] { 0 };
		
		//fs.get(file_data, WAVETABLE_SIZE * sizeof(float));
		// Load float data as bytes
		fs.read(file_data, WAVETABLE_SIZE_BYTES);

		if (fs.fail()) {
			printf("Failed to read data from file '%s'\n", path.string().c_str());
			fs.close();
			return -1;
			// wt.data is automatically deleted by ~Wavetable()
		}

		wt->set_data((float*)file_data);

		// Get wt filename (without extension)
		std::string wavetable_name = path.stem().string();
		if (wavetable_name.empty()) {
			wavetable_name = "NAME_ERROR";
		}

		int name_length = (wavetable_name.length() > MAX_WAVETABLE_NAME_LENGTH) ?
			MAX_WAVETABLE_NAME_LENGTH : wavetable_name.length();

		// Set name
		wt->set_name(new char[MAX_WAVETABLE_NAME_LENGTH] {0});
		memcpy(wt->name, wavetable_name.c_str(), sizeof(char) * name_length);
		wavetables.push_back(wt);
		fs.close();
		printf("Loaded wavetable: '%s'\n", wt->name);

		return wavetables.size() - 1;
	}

	bool WavetableCollection::unload_wavetable(WavetableID id) {
		Wavetable* wt;
		if (!get_wavetable(id, &wt))
			return false;

		if (wt->get_ref() > 0)
			return false;

		wavetables.erase(wavetables.begin() + id);
		return true;
	}

	bool WavetableCollection::get_wavetable(_In_ WavetableID id, _Out_ Wavetable** wavetable) {
		if (id >= wavetables.size())
			return false;
		*wavetable = wavetables[id];
		wavetables[id]->add_ref();
		return true;
	}

	bool WavetableCollection::find_wavetable(_In_ const char* wt_name, _Out_ Wavetable** wavetable) {
		for (int i = 0; i < wavetables.size(); i++) {
			if (strcmp(wavetables[i]->name, wt_name) == 0) {
				*wavetable = wavetables[i];
				return true;
			}
		}
		return false;
	}
}

wavetable::WavetableCollection* load_wavetable_library() {
	wavetable::WavetableCollection* wt_lib = new wavetable::WavetableCollection();
	std::filesystem::path wt_root("./wavetables/");
	int num_loaded = wt_lib->load_folder(wt_root);
	if (num_loaded >= 0) {
		printf("Wavetable library load success: %d wavetables loaded.\n", num_loaded);
	}
	else {
		printf("Failed to load wavetable library.\n");
		delete wt_lib;
		return nullptr;
	}
	return wt_lib;
}

namespace wt = wavetable;
bool generate_wavetables() {
	bool sine_msf = wt::sine::create_wt_file();
	printf("Creating sine... %s.\n", sine_msf ? "success" : "failed");
	bool square_msf = wt::square::create_wt_file();
	printf("Creating square... %s.\n", square_msf ? "success" : "failed");
	bool saw_msf = wt::saw::create_wt_file();
	printf("Creating saw... %s.\n", saw_msf ? "success" : "failed");
	return sine_msf && square_msf && saw_msf;
}