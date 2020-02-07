#pragma once
namespace llvmes {
	namespace graphics {

		enum class EventType {
			WindowCloseEvent, WindowResizeEvent, DropEvent, KeyTypeEvent,
			KeyReleaseEvent, KeyPressEvent, MouseReleaseEvent, MousePressEvent,
			MouseScrollEvent, MouseMoveEvent
		};

		struct KeyPressEvent {
			int keycode;
		};

		struct KeyReleaseEvent {
			int keycode;
		};

		struct Event {
			union {
				KeyPressEvent KeyPress;
				KeyReleaseEvent KeyRelease;
			};
			EventType type;
		};

	}
}