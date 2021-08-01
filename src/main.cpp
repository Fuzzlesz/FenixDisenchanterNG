extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);

	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

template <typename T>
bool isEnchanted(T* a) {
	return a && a->formEnchanting && !a->HasKeyword(0x000C27BD);
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
