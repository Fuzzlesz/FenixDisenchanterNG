
void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		//stl::report_and_fail("Failed to find standard logging directory"sv); // Doesn't work in VR
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;
	v.PluginVersion(0);
	v.PluginName(Version::PROJECT);
	v.AuthorName("");
	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST_AE });
	v.UsesNoStructs(true);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	InitializeLog();

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	return true;
}

template <typename T>
bool isEnchanted(T* a) {
	return a && a->formEnchanting && !a->HasKeyword(RE::TESForm::LookupByID(0x000C27BD)->As<RE::BGSKeyword>());
}

RE::TESObjectWEAP* get_template(RE::TESObjectWEAP* w) { return w->templateWeapon; }
RE::TESObjectARMO* get_template(RE::TESObjectARMO* w) { return w->templateArmor; }

template <typename T>
RE::TESBoundObject* Disenchant_(T* a) {
	if (!isEnchanted(a)) return nullptr;
	for (int i = 0; i < 100; ++i) {
		a = get_template(a);
		if (!a) return nullptr;  // dont know base
		if (!isEnchanted(a)) return a;
	}
	return nullptr;  // mb loop in templates
}

RE::TESForm* Disenchant(RE::StaticFunctionTag*, RE::TESForm* a) {
	auto ans =  Disenchant_(a->As<RE::TESObjectWEAP>());
	if (ans) return ans;
	return Disenchant_(a->As<RE::TESObjectARMO>());
}

bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm)
{
	a_vm->RegisterFunction("DisenchantNative", "f314FD_Utils", Disenchant);

	return true;
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{

	SKSE::Init(a_skse);
	
	auto papyrus = SKSE::GetPapyrusInterface();
	if (!papyrus->Register(RegisterFuncs)) {
		return false;
	}

	return true;
}
