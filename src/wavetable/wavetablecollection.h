#pragma once
#include <fstream>
#include <vector>
#include <filesystem>
#include <string>
#include <iostream>
#include "wavetable.h"


#define WAVETABLE_EXTENSION ".wt"

namespace wavetable {

	namespace fsys = std::filesystem;
	const unsigned int MAX_LIBRARY_SIZE = 512;

	typedef unsigned int WavetableID;

	// A library of wavetables
	class WavetableCollection {
	public:
		// Constructs an empty library
		WavetableCollection() {};

		// Releases all memory for each loaded wavetable
		~WavetableCollection();

		// Loads a library from a given folder (calls load_wavetable on each .wt file in given dir)
		// Returns the number of wavetables loaded,
		// If operation fails, returns -1
		int load_folder(fsys::path const& root);

		// Loads a single wavetable from a file and adds it to the library
		// Returns the allocated id of the new wavetable
		// If it fails to load, returns -1
		int load_wavetable(fsys::path const& file_path);

		// Unloads the specified wavetable from the library
		// Returns true if unloaded
		// Does not unload if someone still has a reference to it
		bool unload_wavetable(WavetableID id);

		// Gets a pointer to the specified wavetable
		// Returns true if found a wavetable of that id
		bool get_wavetable(_In_ WavetableID id, _Out_ Wavetable** wavetable);

		// Gets a pointer to the specified wavetable
		// Returns true if found a wavetable of that name
		bool find_wavetable(_In_ const char* wt_name, _Out_ Wavetable** wavetable);

		static WavetableCollection& get_instance() { return instance; }

	private:
		std::vector<Wavetable*> wavetables;
		static WavetableCollection& instance;
	};
}

wavetable::WavetableCollection* load_wavetable_library();
