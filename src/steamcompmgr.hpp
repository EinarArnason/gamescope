#include <stdint.h>

extern "C" {
#include <wlr/types/wlr_buffer.h>
#include <wlr/render/wlr_texture.h>
}

extern uint32_t currentOutputWidth;
extern uint32_t currentOutputHeight;

unsigned int get_time_in_milliseconds(void);
uint64_t get_time_in_nanos();
void sleep_for_nanos(uint64_t nanos);
void sleep_until_nanos(uint64_t nanos);
timespec nanos_to_timespec( uint64_t ulNanos );

void steamcompmgr_main(int argc, char **argv);

#include "rendervulkan.hpp"
#include "wlserver.hpp"
#include "vblankmanager.hpp"

#include <mutex>
#include <vector>

#include <wlr/render/dmabuf.h>

#include <X11/extensions/Xfixes.h>

struct _XDisplay;
struct steamcompmgr_win_t;
struct xwayland_ctx_t;
class gamescope_xwayland_server_t;

static const uint32_t g_zposBase = 0;
static const uint32_t g_zposOverride = 1;
static const uint32_t g_zposExternalOverlay = 2;
static const uint32_t g_zposOverlay = 3;
static const uint32_t g_zposCursor = 4;
static const uint32_t g_zposMuraCorrection = 5;

extern bool g_bHDRItmEnable;
extern bool g_bForceHDRSupportDebug;

extern EStreamColorspace g_ForcedNV12ColorSpace;

// Disable partial composition for now until we get
// composite priorities working in libliftoff + also
// use the proper libliftoff composite plane system.
static constexpr bool kDisablePartialComposition = true;

struct CursorBarrierInfo
{
	int x1 = 0;
	int y1 = 0;
	int x2 = 0;
	int y2 = 0;
};

struct CursorBarrier
{
	PointerBarrier obj = None;
	CursorBarrierInfo info = {};
};

class MouseCursor
{
public:
	explicit MouseCursor(xwayland_ctx_t *ctx);

	int x() const;
	int y() const;

	void move(int x, int y);
	void constrainPosition();

	void paint(steamcompmgr_win_t *window, steamcompmgr_win_t *fit, FrameInfo_t *frameInfo);
	void setDirty();

	// Will take ownership of data.
	bool setCursorImage(char *data, int w, int h, int hx, int hy);
	bool setCursorImageByName(const char *name);

	void hide() { m_lastMovedTime = 0; checkSuspension(); }

	bool isHidden() { return m_hideForMovement || m_imageEmpty; }
	bool imageEmpty() const { return m_imageEmpty; }

	void forcePosition(int x, int y)
	{
		warp(x, y);
		m_x = x;
		m_y = y;
	}

	void undirty() { getTexture(); }

	xwayland_ctx_t *getCtx() const { return m_ctx; }

	bool needs_server_flush() const { return m_needs_server_flush; }
	void inform_flush() { m_needs_server_flush = false; }

	void GetDesiredSize( int& nWidth, int &nHeight );

	void UpdateXInputMotionMasks();
	void UpdatePosition();

	void checkSuspension();
private:
	void warp(int x, int y);

	bool getTexture();

	void updateCursorFeedback( bool bForce = false );

	int m_x = 0, m_y = 0;
	int m_hotspotX = 0, m_hotspotY = 0;

	std::shared_ptr<CVulkanTexture> m_texture;
	bool m_dirty;
	bool m_imageEmpty;

	unsigned int m_lastMovedTime = 0;
	bool m_hideForMovement;

	bool m_bMotionMaskEnabled = false;

	CursorBarrier m_barriers[4] = {};

	xwayland_ctx_t *m_ctx;

	int m_lastX = 0;
	int m_lastY = 0;

	bool m_bCursorVisibleFeedback = false;
	bool m_needs_server_flush = false;
};

extern std::vector< wlr_surface * > wayland_surfaces_deleted;

extern bool hasFocusWindow;

// These are used for touch scaling, so it's really the window that's focused for touch
extern float focusedWindowScaleX;
extern float focusedWindowScaleY;
extern float focusedWindowOffsetX;
extern float focusedWindowOffsetY;

extern bool g_bFSRActive;

extern uint32_t inputCounter;
extern uint64_t g_lastWinSeq;

void nudge_steamcompmgr( void );
void force_repaint( void );

extern void mangoapp_update( uint64_t visible_frametime, uint64_t app_frametime_ns, uint64_t latency_ns );
gamescope_xwayland_server_t *steamcompmgr_get_focused_server();
struct wlr_surface *steamcompmgr_get_server_input_surface( size_t idx );
wlserver_vk_swapchain_feedback* steamcompmgr_get_base_layer_swapchain_feedback();

struct wlserver_x11_surface_info *lookup_x11_surface_info_from_xid( gamescope_xwayland_server_t *xwayland_server, uint32_t xid );

extern gamescope::VBlankTime g_SteamCompMgrVBlankTime;
extern pid_t focusWindow_pid;

void init_xwayland_ctx(uint32_t serverId, gamescope_xwayland_server_t *xwayland_server);
void gamescope_set_selection(std::string contents, int selection);

MouseCursor *steamcompmgr_get_current_cursor();
MouseCursor *steamcompmgr_get_server_cursor(uint32_t serverId);

extern int g_nAsyncFlipsEnabled;

extern void steamcompmgr_set_app_refresh_cycle_override( gamescope::GamescopeScreenType type, int override_fps );
