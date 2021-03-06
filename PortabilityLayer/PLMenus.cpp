#include "PLMenus.h"
#include "PLResources.h"

#include "MenuManager.h"
#include "QDManager.h"
#include "ResourceManager.h"

// Menu resource structure:
// uint16 menu ID
// uint16 width
// uint16 height
// uint16 command ID
// uint16 padding
// uint32 enable flags
// pstr title (Apple menu is 0x14)
// Zero-byte terminated list:
//     pstr Item text (separator is char 0x2d)
//     int8 Icon #
//     uint8 key
//     uint8 submenu ID
//     uint8 text style

MenuHandle GetMenu(int resID)
{
	Handle menuRes = PortabilityLayer::ResourceManager::GetInstance()->GetAppResource('MENU', resID);

	if (!menuRes)
		return nullptr;

	const MenuHandle menu = PortabilityLayer::MenuManager::GetInstance()->DeserializeMenu(*menuRes);
	menuRes.Dispose();

	return menu;
}

void InsertMenu(MenuHandle menu, int beforeID)
{
	PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
	if (beforeID)
	{
		MenuHandle otherMenu = mm->GetMenuByID(beforeID);
		if (otherMenu)
		{
			mm->InsertMenuBefore(menu, otherMenu);
			return;
		}
	}

	mm->InsertMenuAtEnd(menu);
}

void EnableMenuItem(MenuHandle menu, int index)
{
	PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
	if (index == 0)
		mm->SetMenuEnabled(menu, true);
	else
		mm->SetItemEnabled(menu, index - 1, true);
}

void DisableMenuItem(MenuHandle menu, int index)
{
	PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
	if (index == 0)
		mm->SetMenuEnabled(menu, false);
	else
		mm->SetItemEnabled(menu, index - 1, false);
}

void CheckMenuItem(MenuHandle menu, int index, Boolean checked)
{
	PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
	mm->SetItemChecked(menu, index - 1, checked != 0);
}


void AppendMenuItem(MenuHandle menu, int8_t iconResID, uint8_t key, uint8_t submenuID, uint8_t textStyle, bool enabled, bool checked, const PLPasStr &text)
{
	PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
	mm->AppendMenuItem(menu, iconResID, key, submenuID, textStyle, enabled, checked, text);
}

void SetMenuItemText(MenuHandle menu, int index, const PLPasStr &text)
{
	PortabilityLayer::MenuManager *mm = PortabilityLayer::MenuManager::GetInstance();
	mm->SetItemText(menu, index - 1, text);
}
