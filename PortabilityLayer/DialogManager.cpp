#include "DialogManager.h"
#include "HostDisplayDriver.h"
#include "IconLoader.h"
#include "ResourceManager.h"
#include "PLArrayView.h"
#include "PLBigEndian.h"
#include "PLButtonWidget.h"
#include "PLDialogs.h"
#include "PLIconWidget.h"
#include "PLInvisibleWidget.h"
#include "PLLabelWidget.h"
#include "PLPasStr.h"
#include "PLSysCalls.h"
#include "PLTimeTaggedVOSEvent.h"
#include "PLWidgets.h"
#include "QDPixMap.h"
#include "ResTypeID.h"
#include "SharedTypes.h"
#include "WindowDef.h"
#include "WindowManager.h"

#include <stdlib.h>
#include <new>

namespace PortabilityLayer
{
	class DialogImpl;
	class Widget;

	namespace SerializedDialogItemTypeCodes
	{
		enum SerializedDialogItemTypeCode
		{
			kUserItem = 0x00,
			kButton = 0x04,
			kCheckBox = 0x05,
			kRadioButton = 0x06,
			kCustomControl = 0x07,
			kLabel = 0x08,
			kEditBox = 0x10,
			kIcon = 0x20,
			kImage = 0x40,
		};
	}

	typedef SerializedDialogItemTypeCodes::SerializedDialogItemTypeCode SerializedDialogItemTypeCode_t;


	struct DialogTemplateItem
	{
		Rect m_rect;
		int16_t m_id;
		uint8_t m_serializedType;
		bool m_enabled;
		Str255 m_name;
	};

	class DialogTemplate final
	{
	public:
		DialogTemplate(DialogTemplateItem *itemStorage, size_t numItems);
		void DeserializeItems(const uint8_t *data);
		void Destroy();

		ArrayView<const DialogTemplateItem> GetItems() const;

	private:
		DialogTemplateItem *m_items;
		size_t m_numItems;
	};

	class DialogImpl final : public Dialog
	{
	public:
		void Destroy() override;

		Window *GetWindow() const override;
		ArrayView<const DialogItem> GetItems() const override;

		int16_t ExecuteModal(DialogFilterFunc_t filterFunc) override;

		bool Populate(DialogTemplate *tmpl);

		void DrawControls();

		Point MouseToDialog(const GpMouseInputEvent &evt);

		static DialogImpl *Create(Window *window, size_t numItems);

	private:
		explicit DialogImpl(Window *window, DialogItem *items, size_t numItems);
		~DialogImpl();

		Window *m_window;
		DialogItem *m_items;
		size_t m_numItems;
		size_t m_maxItems;
	};


	DialogItem::DialogItem(Widget *widget)
		: m_widget(widget)
	{
	}

	DialogItem::~DialogItem()
	{
		if (m_widget)
			m_widget->Destroy();
	}

	Widget *DialogItem::GetWidget() const
	{
		return m_widget;
	}

	DialogTemplate::DialogTemplate(DialogTemplateItem *itemStorage, size_t numItems)
		: m_items(itemStorage)
		, m_numItems(numItems)
	{
	}

	void DialogTemplate::DeserializeItems(const uint8_t *data)
	{
		for (size_t i = 0; i < m_numItems; i++)
		{
			BERect itemRect;
			uint8_t itemType;

			data += 4;	// Unused

			memcpy(&itemRect, data, 8);
			data += 8;

			itemType = *data;
			data++;

			uint8_t nameLength = *data;
			data++;

			const uint8_t *nameBytes = data;

			size_t nameLengthPadded = nameLength;
			if ((nameLength & 1) == 1)
				nameLengthPadded++;

			data += nameLengthPadded;

			DialogTemplateItem &item = m_items[i];
			item.m_rect = itemRect.ToRect();
			item.m_id = 0;
			item.m_serializedType = (itemType & 0x7f);
			item.m_enabled = ((itemType & 0x80) == 0);
			item.m_name[0] = nameLength;
			memcpy(item.m_name + 1, nameBytes, nameLength);

			if (item.m_serializedType == SerializedDialogItemTypeCodes::kCustomControl || item.m_serializedType == SerializedDialogItemTypeCodes::kImage || item.m_serializedType == SerializedDialogItemTypeCodes::kIcon)
			{
				memcpy(&item.m_id, item.m_name + 1, 2);
				ByteSwap::BigInt16(item.m_id);
			}
		}
	}

	void DialogTemplate::Destroy()
	{
		this->~DialogTemplate();
		free(this);
	}

	ArrayView<const DialogTemplateItem> DialogTemplate::GetItems() const
	{
		return ArrayView<const DialogTemplateItem>(m_items, m_numItems);
	}

	void DialogImpl::Destroy()
	{
		PortabilityLayer::WindowManager::GetInstance()->DestroyWindow(m_window);

		this->~DialogImpl();
		free(this);
	}

	Window *DialogImpl::GetWindow() const
	{
		return m_window;
	}

	ArrayView<const DialogItem> DialogImpl::GetItems() const
	{
		return ArrayView<const DialogItem>(m_items, m_numItems);
	}

	int16_t DialogImpl::ExecuteModal(DialogFilterFunc_t filterFunc)
	{
		Window *window = this->GetWindow();
		Widget *capturingWidget = nullptr;
		size_t capturingWidgetIndex = 0;

		for (;;)
		{
			TimeTaggedVOSEvent evt;
			if (WaitForEvent(&evt, 1))
			{
				const int16_t selection = filterFunc(this, evt);

				if (selection >= 0)
					return selection;

				if (capturingWidget != nullptr)
				{
					const WidgetHandleState_t state = capturingWidget->ProcessEvent(evt);

					if (state != WidgetHandleStates::kDigested)
						capturingWidget = nullptr;

					if (state == WidgetHandleStates::kActivated)
						return static_cast<int16_t>(capturingWidgetIndex + 1);
				}
				else
				{
					const size_t numItems = this->m_numItems;
					for (size_t i = 0; i < numItems; i++)
					{
						Widget *widget = this->m_items[i].GetWidget();

						const WidgetHandleState_t state = widget->ProcessEvent(evt);

						if (state == WidgetHandleStates::kActivated)
							return static_cast<int16_t>(i + 1);

						if (state == WidgetHandleStates::kCaptured)
						{
							capturingWidget = widget;
							capturingWidgetIndex = i;
							break;
						}

						if (state == WidgetHandleStates::kDigested)
							break;
					}
				}
			}
		}
	}

	bool DialogImpl::Populate(DialogTemplate *tmpl)
	{
		Window *window = this->GetWindow();

		ArrayView<const DialogTemplateItem> templateItems = tmpl->GetItems();

		const size_t numItems = templateItems.Count();

		for (size_t i = 0; i < numItems; i++)
		{
			const DialogTemplateItem &templateItem = templateItems[i];

			Widget *widget = nullptr;

			WidgetBasicState basicState;
			basicState.m_enabled = templateItem.m_enabled;
			basicState.m_resID = templateItem.m_id;
			basicState.m_text = PascalStr<255>(PLPasStr(templateItem.m_name));
			basicState.m_rect = templateItem.m_rect;
			basicState.m_window = window;

			switch (templateItem.m_serializedType)
			{
			case SerializedDialogItemTypeCodes::kButton:
				widget = ButtonWidget::Create(basicState);
				break;
			case SerializedDialogItemTypeCodes::kLabel:
				widget = LabelWidget::Create(basicState);
				break;
			case SerializedDialogItemTypeCodes::kIcon:
				widget = IconWidget::Create(basicState);
				break;
			case SerializedDialogItemTypeCodes::kCheckBox:
			case SerializedDialogItemTypeCodes::kRadioButton:
			case SerializedDialogItemTypeCodes::kEditBox:
			case SerializedDialogItemTypeCodes::kImage:
			default:
				widget = InvisibleWidget::Create(basicState);
				break;
			}

			if (!widget)
				return false;

			new (&m_items[m_numItems++]) DialogItem(widget);
		}

		return true;
	}

	void DialogImpl::DrawControls()
	{
		DrawSurface *surface = m_window->GetDrawSurface();

		for (ArrayViewIterator<const DialogItem> it = GetItems().begin(), itEnd = GetItems().end(); it != itEnd; ++it)
		{
			const DialogItem &item = *it;
			item.GetWidget()->DrawControl(surface);
		}
	}

	Point DialogImpl::MouseToDialog(const GpMouseInputEvent &evt)
	{
		const Window *window = m_window;
		const int32_t x = evt.m_x - window->m_wmX;
		const int32_t y = evt.m_y - window->m_wmY;

		return Point::Create(x, y);
	}

	DialogImpl *DialogImpl::Create(Window *window, size_t numItems)
	{
		size_t alignedSize = sizeof(DialogImpl) + GP_SYSTEM_MEMORY_ALIGNMENT + 1;
		alignedSize -= alignedSize % GP_SYSTEM_MEMORY_ALIGNMENT;

		const size_t itemsSize = sizeof(DialogItem) * numItems;

		void *storage = malloc(alignedSize + itemsSize);
		if (!storage)
			return nullptr;

		DialogItem *itemsList = reinterpret_cast<DialogItem *>(static_cast<uint8_t*>(storage) + alignedSize);

		return new (storage) DialogImpl(window, itemsList, numItems);
	}

	DialogImpl::DialogImpl(Window *window, DialogItem *itemsList, size_t numItems)
		: m_window(window)
		, m_items(itemsList)
		, m_numItems(0)
		, m_maxItems(numItems)
	{
	}

	DialogImpl::~DialogImpl()
	{
		while (m_numItems > 0)
		{
			m_numItems--;
			m_items[m_numItems].~DialogItem();
		}
	}

	// DLOG resource format:
	// DialogResourceDataHeader
	// Variable-length PStr: Title
	// Optional: Positioning (2 byte mask)

	struct DialogResourceDataHeader
	{
		BERect m_rect;
		BEInt16_t m_style;
		uint8_t m_visible;
		uint8_t m_unusedA;
		uint8_t m_hasCloseBox;
		uint8_t m_unusedB;
		BEUInt32_t m_referenceConstant;
		BEInt16_t m_itemsResID;
	};

	class DialogManagerImpl final : public DialogManager
	{
	public:
		Dialog *LoadDialog(int16_t resID, Window *behindWindow) override;
		DialogTemplate *LoadDialogTemplate(int16_t resID);

		static DialogManagerImpl *GetInstance();

	private:
		static DialogManagerImpl ms_instance;
	};

	Dialog *DialogManagerImpl::LoadDialog(int16_t resID, Window *behindWindow)
	{
		ResourceManager *rm = ResourceManager::GetInstance();

		THandle<uint8_t> dlogH = rm->GetResource('DLOG', resID).StaticCast<uint8_t>();
		const uint8_t *dlogData = *dlogH;
		const uint8_t *dlogDataEnd = dlogData + dlogH.MMBlock()->m_size;

		DialogResourceDataHeader header;
		memcpy(&header, dlogData, sizeof(header));

		const uint8_t *titlePStr = dlogData + sizeof(header);
		const uint8_t *positioningData = titlePStr + 1 + titlePStr[0];	// May be OOB

		BEUInt16_t positionSpec(0);
		if (positioningData != dlogDataEnd)
			memcpy(&positionSpec, positioningData, 2);

		const Rect rect = header.m_rect.ToRect();
		const int16_t style = header.m_style;

		dlogH.Dispose();

		DialogTemplate *dtemplate = LoadDialogTemplate(header.m_itemsResID);
		const size_t numItems = (dtemplate == nullptr) ? 0 : dtemplate->GetItems().Count();

		if (!rect.IsValid())
		{
			dtemplate->Destroy();
			return nullptr;
		}

		WindowManager *wm = PortabilityLayer::WindowManager::GetInstance();

		WindowDef wdef = WindowDef::Create(rect, 0, header.m_visible != 0, header.m_hasCloseBox != 0, header.m_referenceConstant, positionSpec, PLPasStr(titlePStr));
		Window *window = wm->CreateWindow(wdef);
		if (!window)
		{
			dtemplate->Destroy();
			return nullptr;
		}

		wm->PutWindowBehind(window, behindWindow);

		unsigned int displayWidth, displayHeight;
		PortabilityLayer::HostDisplayDriver::GetInstance()->GetDisplayResolution(&displayWidth, &displayHeight, nullptr);

		const unsigned int halfDisplayHeight = displayHeight / 2;
		const unsigned int quarterDisplayWidth = displayHeight / 4;
		const unsigned int halfDisplayWidth = displayWidth;

		const uint16_t dialogWidth = rect.Width();
		const uint16_t dialogHeight = rect.Height();

		window->m_wmX = (static_cast<int32_t>(displayWidth) - static_cast<int32_t>(dialogWidth)) / 2;

		// We center dialogs vertically in one of 3 ways in this priority:
		// - Centered at 1/3 until the top edge is at the 1/4 mark
		// - Top edge aligned to 1/4 mark until bottom edge is at 3/4 mark
		// - Centered on screen

		//if (displayHeight / 3 - dialogHeight / 2 >= displayHeight / 4)
		if (displayHeight * 4 - dialogHeight * 6 >= displayHeight * 3)
		{
			//window->m_wmY = displayHeight / 3 - dialogHeight / 2;
			window->m_wmY = (static_cast<int32_t>(displayHeight * 2) - static_cast<int32_t>(dialogHeight * 3)) / 6;
		}
		else if (dialogHeight * 2 <= displayHeight)
			window->m_wmY = displayHeight / 4;
		else
			window->m_wmY = (static_cast<int32_t>(displayHeight) - static_cast<int32_t>(dialogHeight)) / 2;

		DialogImpl *dialog = DialogImpl::Create(window, numItems);

		if (!dialog)
		{
			dtemplate->Destroy();
			wm->DestroyWindow(window);
			return nullptr;
		}

		if (!dialog->Populate(dtemplate))
		{
			dialog->Destroy();
			dtemplate->Destroy();
			wm->DestroyWindow(window);

			return nullptr;
		}

		dialog->DrawControls();

		return dialog;
	}

	DialogTemplate *DialogManagerImpl::LoadDialogTemplate(int16_t resID)
	{
		ResourceManager *rm = ResourceManager::GetInstance();

		THandle<uint8_t> dtemplateH = rm->GetResource('DITL', resID).StaticCast<uint8_t>();

		if (!dtemplateH)
			return nullptr;

		int16_t numItemsMinusOne;
		memcpy(&numItemsMinusOne, *dtemplateH, 2);
		ByteSwap::BigInt16(numItemsMinusOne);

		if (numItemsMinusOne < -1)
			return nullptr;

		uint16_t numItems = static_cast<uint16_t>(numItemsMinusOne + 1);

		size_t dtlAlignedSize = sizeof(DialogTemplate) + GP_SYSTEM_MEMORY_ALIGNMENT - 1;
		dtlAlignedSize -= dtlAlignedSize % GP_SYSTEM_MEMORY_ALIGNMENT;

		const size_t dtlItemSize = sizeof(DialogTemplateItem) * numItems;

		void *storage = malloc(dtlAlignedSize + dtlItemSize);
		if (!storage)
		{
			dtemplateH.Dispose();
			return nullptr;
		}

		uint8_t *itemsLoc = static_cast<uint8_t*>(storage) + dtlAlignedSize;

		DialogTemplate *dtemplate = new (storage) DialogTemplate(reinterpret_cast<DialogTemplateItem*>(itemsLoc), numItems);
		dtemplate->DeserializeItems((*dtemplateH) + 2);

		dtemplateH.Dispose();

		return dtemplate;
	}

	DialogManagerImpl *DialogManagerImpl::GetInstance()
	{
		return &ms_instance;
	}

	DialogManagerImpl DialogManagerImpl::ms_instance;

	DialogManager *DialogManager::GetInstance()
	{
		return DialogManagerImpl::GetInstance();
	}
}