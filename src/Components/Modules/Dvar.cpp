#include <STDInclude.hpp>

namespace Components
{
	const char* Dvar::ArchiveDvarPath = "userraw/archivedvars.cfg";

	Dvar::Var Dvar::Name;

	Dvar::Var::Var(const std::string& dvarName)
	{
		this->dvar_ = Game::Dvar_FindVar(dvarName.data());

		// If the dvar can't be found it will be registered as an empty string dvar
		if (this->dvar_ == nullptr)
		{
			this->dvar_ = const_cast<Game::dvar_t*>(Game::Dvar_SetFromStringByNameFromSource(dvarName.data(), "", Game::DVAR_SOURCE_INTERNAL));
		}
	}

	template <> Game::dvar_t* Dvar::Var::get()
	{
		return this->dvar_;
	}

	template <> const char* Dvar::Var::get()
	{
		if (this->dvar_ == nullptr)
			return "";

		if (this->dvar_->type == Game::DVAR_TYPE_STRING || this->dvar_->type == Game::DVAR_TYPE_ENUM)
		{
			if (this->dvar_->current.string != nullptr)
				return this->dvar_->current.string;
		}

		return "";
	}

	template <> int Dvar::Var::get()
	{
		if (this->dvar_ == nullptr)
			return 0;

		if (this->dvar_->type == Game::DVAR_TYPE_INT || this->dvar_->type == Game::DVAR_TYPE_ENUM)
		{
			return this->dvar_->current.integer;
		}

		return 0;
	}

	template <> unsigned int Dvar::Var::get()
	{
		if (this->dvar_ == nullptr)
			return 0;

		if (this->dvar_->type == Game::DVAR_TYPE_INT)
		{
			return this->dvar_->current.unsignedInt;
		}

		return 0;
	}

	template <> float Dvar::Var::get()
	{
		if (this->dvar_ == nullptr)
			return 0.f;

		if (this->dvar_->type == Game::DVAR_TYPE_FLOAT)
		{
			return this->dvar_->current.value;
		}

		return 0.f;
	}

	template <> float* Dvar::Var::get()
	{
		static Game::vec4_t vector{0.f, 0.f, 0.f, 0.f};

		if (this->dvar_ == nullptr)
			return vector;

		if (this->dvar_->type == Game::DVAR_TYPE_FLOAT_2 || this->dvar_->type == Game::DVAR_TYPE_FLOAT_3 ||
			this->dvar_->type == Game::DVAR_TYPE_FLOAT_4)
		{
			return this->dvar_->current.vector;
		}

		return vector;
	}

	template <> bool Dvar::Var::get()
	{
		if (this->dvar_ == nullptr)
			return false;

		if (this->dvar_->type == Game::DVAR_TYPE_BOOL)
		{
			return this->dvar_->current.enabled;
		}

		return false;
	}

	template <> std::string Dvar::Var::get()
	{
		return this->get<const char*>();
	}

	void Dvar::Var::set(const char* string)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_STRING);
		if (this->dvar_)
		{
			Game::Dvar_SetString(this->dvar_, string);
		}
	}

	void Dvar::Var::set(const std::string& string)
	{
		this->set(string.data());
	}

	void Dvar::Var::set(int integer)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_INT);

		if (this->dvar_)
		{
			Game::Dvar_SetInt(this->dvar_, integer);
		}
	}

	void Dvar::Var::set(float value)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_FLOAT);

		if (this->dvar_)
		{
			Game::Dvar_SetFloat(this->dvar_, value);
		}
	}

	void Dvar::Var::set(bool enabled)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_BOOL);

		if (this->dvar_)
		{
			Game::Dvar_SetBool(this->dvar_, enabled);
		}
	}

	void Dvar::Var::setRaw(int integer)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_INT);

		if (this->dvar_)
		{
			this->dvar_->current.integer = integer;
			this->dvar_->latched.integer = integer;
		}
	}

	void Dvar::Var::setRaw(float value)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_FLOAT);

		if (this->dvar_)
		{
			this->dvar_->current.value = value;
			this->dvar_->latched.value = value;
		}
	}

	void Dvar::Var::setRaw(bool enabled)
	{
		assert(this->dvar_->type == Game::DVAR_TYPE_BOOL);

		if (this->dvar_)
		{
			this->dvar_->current.enabled = enabled;
			this->dvar_->latched.enabled = enabled;
		}
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, bool value, Flag flag, const char* description)
	{
		return Game::Dvar_RegisterBool(dvarName, value, flag.val, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, const char* value, Flag flag, const char* description)
	{
		return Game::Dvar_RegisterString(dvarName, value, flag.val, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, int value, int min, int max, Flag flag, const char* description)
	{
		return Game::Dvar_RegisterInt(dvarName, value, min, max, flag.val, description);
	}

	template<> Dvar::Var Dvar::Register(const char* dvarName, float value, float min, float max, Flag flag, const char* description)
	{
		return Game::Dvar_RegisterFloat(dvarName, value, min, max, flag.val, description);
	}

	void Dvar::ResetDvarsValue()
	{
		if (!Utils::IO::FileExists(ArchiveDvarPath))
			return;

		Command::Execute("exec archivedvars.cfg", true);
		// Clean up
		Utils::IO::RemoveFile(ArchiveDvarPath);
	}

	Game::dvar_t* Dvar::Dvar_RegisterName(const char* name, const char* /*default*/, std::uint16_t flags, const char* description)
	{
		// Name watcher
		if (!Dedicated::IsEnabled() && !ZoneBuilder::IsEnabled())
		{
			Scheduler::Loop([]
			{
				static std::string lastValidName = "Unknown Soldier";
				auto name = Name.get<std::string>();

				// Don't perform any checks if name didn't change
				if (name == lastValidName) return;

				std::string saneName = TextRenderer::StripAllTextIcons(TextRenderer::StripColors(Utils::String::Trim(name)));
				if (saneName.size() < 3 || (saneName[0] == '[' && saneName[1] == '{'))
				{
					Logger::PrintError(Game::CON_CHANNEL_ERROR, "Username '{}' is invalid. It must at least be 3 characters long and not appear empty!\n", name);
					Name.set(lastValidName);
				}
				else
				{
					lastValidName = name;
					Friends::UpdateName();
				}
			}, Scheduler::CLIENT, 3s); // Don't need to do this every frame
		}

		std::string username = "Unknown Soldier";

		if (Steam::Proxy::SteamFriends)
		{
			const char* steamName = Steam::Proxy::SteamFriends->GetPersonaName();

			if (steamName && *steamName != '\0')
			{
				username = steamName;
			}
		}

		Name = Register<const char*>(name, username.data(), flags | Game::DVAR_ARCHIVE, description);
		return Name.get<Game::dvar_t*>();
	}

	void Dvar::SetFromStringByNameSafeExternal(const char* dvarName, const char* string)
	{
		static std::array<const char*, 8> exceptions =
		{
			"ui_showEndOfGame",
			"systemlink",
			"splitscreen",
			"onlinegame",
			"party_maxplayers",
			"xblive_privateserver",
			"xblive_rankedmatch",
			"ui_mptype",
		};

		for (const auto& entry : exceptions)
		{
			if (Utils::String::Compare(dvarName, entry))
			{
				Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DVAR_SOURCE_INTERNAL);
				return;
			}
		}

		SetFromStringByNameExternal(dvarName, string);
	}

	void Dvar::SetFromStringByNameExternal(const char* dvarName, const char* string)
	{
		Game::Dvar_SetFromStringByNameFromSource(dvarName, string, Game::DVAR_SOURCE_EXTERNAL);
	}

	bool Dvar::AreArchiveDvarsProtected()
	{
		static std::optional<bool> flag;

		if (!flag.has_value())
		{
			flag.emplace(Flags::HasFlag("protect-saved-dvars"));
		}

		return flag.value();
	}

	void Dvar::SaveArchiveDvar(const Game::dvar_t* var)
	{
		if (!Utils::IO::FileExists(ArchiveDvarPath))
		{
			Utils::IO::WriteFile(ArchiveDvarPath, "// generated by IW4x, do not modify\n");
		}

		Utils::IO::WriteFile(ArchiveDvarPath, Utils::String::VA("seta %s \"%s\"\n", var->name, Game::Dvar_DisplayableValue(var)), true);
	}

	void Dvar::DvarSetFromStringByName_Stub(const char* dvarName, const char* value)
	{
		// Save the dvar original value if it has the archive flag
		const auto* dvar = Game::Dvar_FindVar(dvarName);
		if (dvar != nullptr && dvar->flags & Game::DVAR_ARCHIVE)
		{
			if (AreArchiveDvarsProtected())
			{
				Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Not allowing server to override saved dvar '{}'\n", dvarName);
				return;
			}

#ifdef DEBUG_DVARS
			Logger::Print(Game::CON_CHANNEL_CONSOLEONLY, "Server is overriding saved dvar '{}'\n", dvarName);
#endif
			SaveArchiveDvar(dvar);
		}

		Utils::Hook::Call<void(const char*, const char*)>(0x4F52E0)(dvarName, value);
	}

	void Dvar::OnRegisterVariant([[maybe_unused]] Game::dvar_t* dvar)
	{
#ifdef _DEBUG
		dvar->flags &= ~Game::DVAR_CHEAT;
#endif
	}

	__declspec(naked) void Dvar::Dvar_RegisterVariant_Stub()
	{
		__asm
		{
			pushad

			push eax
			call OnRegisterVariant
			add esp, 0x4

			popad

			// Game's code
			pop edi
			pop esi
			pop ebp
			pop ebx
			ret
		}
	}

	const char* Dvar::Dvar_EnumToString_Stub(const Game::dvar_t* dvar)
	{
		assert(dvar);
		assert(dvar->name);
		assert(dvar->type == Game::DVAR_TYPE_ENUM);
		assert(dvar->domain.enumeration.strings);
		assert(dvar->current.integer >= 0 && dvar->current.integer < dvar->domain.enumeration.stringCount || dvar->current.integer == 0);

		// Fix nullptr crash
		if (!dvar || dvar->domain.enumeration.stringCount == 0)
		{
			return "";
		}

		return dvar->domain.enumeration.strings[dvar->current.integer];
	}

	Dvar::Dvar()
	{
		// set flags of cg_drawFPS to archive
		Utils::Hook::Or<std::uint8_t>(0x4F8F69, Game::DVAR_ARCHIVE);

		// un-cheat camera_thirdPersonCrosshairOffset and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x447B41, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);
		
		// un-cheat cg_fov and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x4F8E35, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);
		
		// un-cheat cg_fovscale and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x4F8E68, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

		// un-cheat cg_debugInfoCornerOffset and add archive flags
		Utils::Hook::Xor<std::uint8_t>(0x4F8FC2, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE);

		// remove archive flags for cg_hudchatposition
		Utils::Hook::Xor<std::uint8_t>(0x4F9992, Game::DVAR_ARCHIVE);

		// remove write protection from fs_game
		Utils::Hook::Xor<std::uint32_t>(0x6431EA, Game::DVAR_INIT);

		// set cg_fov max to 160.0
		// because that's the max on SP
		static float cg_Fov = 160.0f;
		Utils::Hook::Set<float*>(0x4F8E28, &cg_Fov);

		// set max volume to 1
		static float volume = 1.0f;
		Utils::Hook::Set<float*>(0x408078, &volume);

		// Uncheat ui_showList
		Utils::Hook::Xor<std::uint8_t>(0x6310DC, Game::DVAR_CHEAT);

		// Uncheat ui_debugMode
		Utils::Hook::Xor<std::uint8_t>(0x6312DE, Game::DVAR_CHEAT);

		// Hook dvar 'name' registration
		Utils::Hook(0x40531C, Dvar_RegisterName, HOOK_CALL).install()->quick();

		// un-cheat safeArea_* and add archive flags
		Utils::Hook::Xor<std::uint32_t>(0x42E3F5, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_horizontal
		Utils::Hook::Xor<std::uint32_t>(0x42E423, Game::DVAR_ROM | Game::DVAR_ARCHIVE); //safeArea_adjusted_vertical
		Utils::Hook::Xor<std::uint8_t>(0x42E398, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_horizontal
		Utils::Hook::Xor<std::uint8_t>(0x42E3C4, Game::DVAR_CHEAT | Game::DVAR_ARCHIVE); //safeArea_vertical

		// Don't allow setting cheat protected dvars via menus
		Utils::Hook(0x63C897, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x63CA96, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x63CDB5, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x635E47, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		// Script_SetDvar
		Utils::Hook(0x63444C, SetFromStringByNameSafeExternal, HOOK_CALL).install()->quick();

		// Slider
		Utils::Hook(0x636159, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636189, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x6364EA, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		Utils::Hook(0x636207, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636608, SetFromStringByNameExternal, HOOK_CALL).install()->quick();
		Utils::Hook(0x636695, SetFromStringByNameExternal, HOOK_CALL).install()->quick();

		// Hook Dvar_SetFromStringByName inside CG_SetClientDvarFromServer so we can reset dvars when the player leaves the server
		Utils::Hook(0x59386A, DvarSetFromStringByName_Stub, HOOK_CALL).install()->quick();

		// If the game closed abruptly, the dvars would not have been restored
		Scheduler::Once(ResetDvarsValue, Scheduler::Pipeline::MAIN);

		// Reset archive dvars when client leaves a server
		Events::OnSteamDisconnect(ResetDvarsValue);

		// For debugging
		Utils::Hook(0x6483FA, Dvar_RegisterVariant_Stub, HOOK_JUMP).install()->quick();
		Utils::Hook(0x648438, Dvar_RegisterVariant_Stub, HOOK_JUMP).install()->quick();

		// Fix crash
		Utils::Hook(0x4B7120, Dvar_EnumToString_Stub, HOOK_JUMP).install()->quick();
	}

	Dvar::~Dvar()
	{
		Utils::IO::RemoveFile(ArchiveDvarPath);
	}
}
