//
// Created by southern_high on 2/7/25.
//
#include <jni.h>
#include <string>
#include <cstring>
#include <android/log.h>
#include <android/bitmap.h>
// LiteHTML includes
#include "litehtml/include/litehtml.h"
#include "litehtml/include/litehtml/document_container.h"
#include "litehtml/src/document.cpp"
#include "litehtml/src/gumbo/include/gumbo.h"

#define LOG_TAG "Southern_CustomWebViewNative"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


// --- LiteHTML Document Container Implementation ---
// LiteHTML requires you to implement a 'document_container' interface.
// This interface provides callbacks for LiteHTML to:
// - Create fonts
// - Get text width
// - Draw backgrounds, text, images, etc.
// - Get client rectangle
// - Set cursor
// - And more...
// This is the most complex part of the C++ integration.

class AndroidDocumentContainer : public litehtml::document_container {
public:
    AndroidBitmapInfo bitmapInfo;
    void* bitmapPixels;
    int viewWidth;
    int viewHeight;

    AndroidDocumentContainer(JNIEnv* env,
                             jobject bitmap,
                             int width,
                             int height) : viewWidth(width), viewHeight(height) {
        if (AndroidBitmap_getInfo(env, bitmap, &bitmapInfo) < 0) {
            LOGE("AndroidBitmap_getInfo() failed!");
            return;
        }
        if (bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
            LOGE("Bitmap format is not RGBA_888!");
            return;
        }
        if (AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels) < 0) {
            LOGE("AndroidBitmap_lockPixels() failed!");
            return;
        }
    }

    ~AndroidDocumentContainer() {
        // Unlock pixels if locked (though usually done after drawing)
    }

    // --- Implement pure virtual functions from litehtml::document_container ---
    litehtml::uint_ptr  create_font(const litehtml::font_description& descr, const litehtml::document* doc, litehtml::font_metrics* fm) override {
        if (fm) {
            fm -> ascent = descr.size * 0.8;
            fm -> descent = descr.size * 0.2;
            fm -> height = descr.size;
            fm -> x_height = descr.size / 2;
            fm -> draw_spaces = true;
        }
        LOGI("create_font: %s, size: %d", descr.family.c_str(), descr.size);
        return 1;
    }

    void delete_font(litehtml::uint_ptr hFont) override {
        LOGI("delete_font: %d", hFont);
    }

    int text_width(const char* text, litehtml::uint_ptr hFont) override {
        if (!text) return 0;
        int len = strlen(text);
        int avg_char_width = 8;
        return len * avg_char_width;
    }

    void draw_text(litehtml::uint_ptr hdc, const char* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos) override {
        LOGI("draw_text: '%s' at (%d, %d) color(%d,%d,%d)", text, pos.x, pos.y, color.red, color.green, color.blue);
        // Example: Drawing a red pixel (highly simplified)
        if (pos.x >= 0 && pos.x < bitmapInfo.width && pos.y >= 0 && pos.y < bitmapInfo.height) {
            uint32_t* pixel = (uint32_t*)((char*)bitmapPixels + pos.y * bitmapInfo.stride) + pos.x;
            // Assuming RGBA_8888, ARGB in memory
            *pixel = (color.alpha << 24) | (color.red << 16) | (color.green << 8) | color.blue;
        }
    }

    int pt_to_px(int pt) const override {
        return (int)((double)pt * 96.0 / 72.0); // Assuming 96 DPI
    }

    int get_default_font_size() const override {
        return 16; // Example default
    }

    const char* get_default_font_name() const override {
        return "sans-serif"; // Example default
    }

    void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker) override {
        // TODO: Implement drawing of list item markers (bullets, numbers).
        LOGI("draw_list_marker at (%d, %d)", marker.pos.x, marker.pos.y);
    }

    void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override {
        // TODO: Implement image loading. This is asynchronous.
        // You'll need to:
        // 1. Construct the full image URL (src + baseurl).
        // 2. Fetch the image (e.g., using JNI to call Android's networking).
        // 3. Decode the image into a format LiteHTML can use or you can draw.
        // 4. If redraw_on_ready is true, trigger a redraw of the view once the image is loaded.
        LOGI("load_image: %s, baseurl: %s", src, baseurl);
    }

    void get_image_size(const char* src, const char* baseurl, litehtml::size& sz) override {
        // TODO: Return the size of the loaded image.
        // If the image isn't loaded yet, you might set sz.width/height to 0
        // or a placeholder size.
        sz.width = 0; // Placeholder
        sz.height = 0; // Placeholder
    }

    void draw_image(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const std::string& url, const std::string& base_url) override {
        // Skip actual drawing for now.
        // You can log:
        LOGI("draw_image called with url: %s", url.c_str());
    }

    void draw_solid_fill(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::web_color& color) override {
        // Skip drawing for now.
        // You can log color:
        LOGI("draw_solid_fill called with color: #%02x%02x%02x", color.red, color.green, color.blue);
    }

    void draw_linear_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::linear_gradient& gradient) override {
        LOGI("draw_linear_gradient called");
    }

    void draw_radial_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::radial_gradient& gradient) override {
        LOGI("draw_radial_gradient called");
    }

    void draw_conic_gradient(litehtml::uint_ptr hdc, const litehtml::background_layer& layer, const litehtml::background_layer::conic_gradient& gradient) override {
        LOGI("draw_conic_gradient called");
    }

    void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders& borders, const litehtml::position& draw_pos, bool root) override {
        // TODO: Implement border drawing.
    }

    void set_caption(const char* caption) override {
        // Not directly applicable to a custom view, maybe log it.
        LOGI("set_caption: %s", caption);
    }

    void set_base_url(const char* base_url) override {
        // Store the base URL if needed for resolving relative paths in load_image etc.
        LOGI("set_base_url: %s", base_url);
    }

    void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override {
        // TODO: Handle stylesheets linked in the HTML (<link rel="stylesheet">).
        // You'd need to load the CSS content and provide it to the LiteHTML document.
    }

    void on_anchor_click(const char* url, const litehtml::element::ptr& el) override {
        // TODO: Handle link clicks. You might want to call back to Java/Kotlin
        // to open the URL in a browser or another WebView.
        LOGI("on_anchor_click: %s", url);
        // JNI call to Java could be made here
    }

    bool on_element_click(const litehtml::element::ptr& el) override {
        const char* href = el->get_attr("href");
        if (href && href[0] != '\0') {
            __android_log_print(ANDROID_LOG_INFO, "LiteHTML", "Clicked on href: %s", href);
            // Optionally call into your Kotlin bridge here.
            return true;
        }
        return false;
    }

    void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override {
        LOGI("Mobile does not support mause");
    }

    void set_cursor(const char* cursor) override {
        // TODO: Potentially change the mouse cursor (less relevant on touch devices).
    }

    void transform_text(litehtml::string& text, litehtml::text_transform tt) override {
        // TODO: Implement text transformations (uppercase, lowercase, capitalize).
        // litehtml::tstring is likely std::string or std::wstring.
    }

    void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override {
        // TODO: Handle @import rules in CSS. Load the content of the imported CSS.
        LOGI("import_css: url=%s, baseurl=%s", url.c_str(), baseurl.c_str());
        // text = "body { background-color: #f0f; }"; // Example: provide imported CSS content
    }

    void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override {
        // TODO: Set a clipping region for drawing.
        // This is important for things like overflow:hidden.
        // You might use Canvas.clipRect or similar via JNI if needed,
        // or manage clipping within your C++ drawing logic.
    }

    void del_clip() override {
        // TODO: Remove the clipping region.
    }

    void get_viewport(litehtml::position& viewport) const override {
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = 1080;   // or your WebView width
        viewport.height = 1920;  // or your WebView height
    }

    litehtml::element::ptr create_element(
            const char* tag_name,
            const litehtml::string_map& attributes,
            const std::shared_ptr<litehtml::document>& doc
    ) override {
        return nullptr; // fallback to default litehtml handling
    }

    void get_media_features(litehtml::media_features& media) const override {
        media.type = litehtml::media_type_screen;
        media.width = 1080;       // viewport width
        media.height = 1920;      // viewport height
        media.device_width = 1080;
        media.device_height = 1920;
        media.color = 8;          // color depth
        media.monochrome = 0;
        media.color_index = 256;
        media.resolution = 96;    // DPI
//        media.orientation = (media.width >= media.height) ? litehtml::media_orientation_landscape : litehtml::media_orientation_portrait;
    }

    void get_language(litehtml::string& language, litehtml::string& culture) const override {
        language = "en";
        culture = "US";
        // Pull from android locale later
    }

    litehtml::string resolve_color(const litehtml::string& color) const override {
        return litehtml::string(); // let litehtml handle
    }

    void split_text(
            const char* text,
            const std::function<void(const char*)>& on_word,
            const std::function<void(const char*)>& on_space
    ) override {
        const char* p = text;
        const char* word_start = p;
        bool in_space = (*p == ' ' || *p == '\t' || *p == '\n');

        while (*p) {
            bool current_is_space = (*p == ' ' || *p == '\t' || *p == '\n');
            if (current_is_space != in_space) {
                std::string segment(word_start, p - word_start);
                if (in_space) {
                    on_space(segment.c_str());
                } else {
                    on_word(segment.c_str());
                }
                word_start = p;
                in_space = current_is_space;
            }
            ++p;
        }
        // handle last segment
        if (p != word_start) {
            std::string segment(word_start, p - word_start);
            if (in_space) {
                on_space(segment.c_str());
            } else {
                on_word(segment.c_str());
            }
        }
    }
};

std::shared_ptr<litehtml::document> doc = nullptr;
AndroidDocumentContainer* global_container = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_example_customweb_webview_CustomWebView_nativeInit(JNIEnv* env,
                                                            jobject  /* this */,
                                                            jint width,
                                                            jint height) {
    LOGI("NativeInit called with width: %d, height: %d", width, height);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_customweb_webview_CustomWebView_nativeRender(JNIEnv* env,
                                                              jobject /* this */,
                                                              jstring html_content_java,
                                                              jobject bitmap) {
    if (!html_content_java || !bitmap) {
        LOGE("HTML content or bitmap is null");
        return;
    }

    const char *html_content_c = env->GetStringUTFChars(html_content_java, nullptr);
    if (!html_content_c) {
        LOGE("Failed to get C string from html_content_java");
        return; // Out of memory or other error
    }
    LOGI("NativeRender called with HTML: %s", html_content_c);


    AndroidBitmapInfo info;
    void *pixels;
    int ret;

    if ((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed! error=%d", ret);
        env->ReleaseStringUTFChars(html_content_java, html_content_c);
        return;
    }

    if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
        LOGE("Bitmap format is not RGBA_8888!");
        env->ReleaseStringUTFChars(html_content_java, html_content_c);
        return;
    }

    // Clean up previous container and document if they exist
    if (global_container) {
        AndroidBitmap_unlockPixels(env, bitmap); // Assuming it was locked by the container
        delete global_container;
        global_container = nullptr;
    }
    doc = nullptr; // Release shared_ptr

    // Create the container BEFORE locking pixels if the container locks them.
    // Or pass env and bitmap to the container for it to lock.
    // For this example, the container locks pixels in its constructor.
    global_container = new AndroidDocumentContainer(env, bitmap, info.width, info.height);
    if (!global_container->bitmapPixels) { // Check if locking failed in constructor
        LOGE("Failed to initialize or lock bitmap in AndroidDocumentContainer");
        delete global_container;
        global_container = nullptr;
        env->ReleaseStringUTFChars(html_content_java, html_content_c);
        return;
    }

    // TODO: Load master CSS if you have one.
    // const char* master_css_c = "body { font-family: sans-serif; }"; // Example
    // litehtml::context ctx;
    // ctx.load_master_stylesheet(master_css_c);
    // doc = litehtml::document::createFromString(html_content_c, global_container, &ctx);

    // Create LiteHTML document
    // You might need a litehtml::context for master CSS, etc.

    if (!global_container) {
        LOGE("global_container is null before createFromString!");
        env->ReleaseStringUTFChars(html_content_java, html_content_c);
        return;
    }
    const std::string master_styles = R"css(
    body, html { display: block; margin: 0; padding: 0; }
    p { display: block; margin: 1em 0; }
    img { display: inline-block; }
    )css";
    const std::string user_styles = "body { background-color: #f0f0f0; }";
//    static litehtml::contex ctx;
    doc = litehtml::document::createFromString(html_content_c, global_container, master_styles, user_styles); // No context, no master CSS for simplicity

    env->ReleaseStringUTFChars(html_content_java, html_content_c);

    if (doc) {
        LOGI("LiteHTML document created. Rendering...");
        // Render the document. The width here is the available width for layout.
        // The height is an initial estimate; LiteHTML might adjust it.
        doc->render(info.width, litehtml::render_all); // Render with available width

        // Draw the document. This will call the draw_* methods in your container.
        // The position {0, 0, info.width, info.height} is the viewport.
        litehtml::position viewport_clip(0, 0, info.width, info.height);
        doc->draw((litehtml::uint_ptr)0, 0, 0, &viewport_clip); // hdc is not used in this container, x, y are initial draw offsets

        LOGI("LiteHTML rendering and drawing finished.");
    } else {
        LOGE("Failed to create LiteHTML document.");
    }

    // Unlock pixels (if locked by the container or here)
    // The AndroidDocumentContainer destructor could also handle this,
    // but it's safer to do it explicitly after drawing.
    if (global_container && global_container->bitmapPixels) {
        AndroidBitmap_unlockPixels(env, bitmap);
        global_container->bitmapPixels = nullptr; // Mark as unlocked
    }
    // Consider deleting global_container here if it's single-use per render,
    // or manage its lifecycle more carefully. For now, it's cleaned up at the start of the next call.
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_example_customweb_webview_CustomWebView_nativeGetDocumentHeight(JNIEnv *env, jobject /* this */) {
    if (doc) {
        return doc->height();
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_example_customweb_webview_CustomWebView_nativeOnTouchEvent(JNIEnv *env, jobject /* this */, jint event_type, jint x, jint y) {
    if (doc && global_container) {
        bool redraw = false;
        litehtml::position::vector redraw_boxes; // ✅ correct type

        switch (event_type) {
            case 0: // ACTION_DOWN
                redraw = doc->on_lbutton_down(x, y, x, y, redraw_boxes);
                break;
            case 1: // ACTION_UP
                redraw = doc->on_lbutton_up(x, y, x, y, redraw_boxes);
                break;
            case 2: // ACTION_MOVE
                redraw = doc->on_mouse_over(x, y, x, y, redraw_boxes);
                break;
        }

        if (redraw) {
            for (const auto& box : redraw_boxes) {
                __android_log_print(
                        ANDROID_LOG_INFO, "LiteHTML",
                        "Redraw box: left=%d, top=%d, width=%d, height=%d",
                        box.left(), box.top(), box.width, box.height
                );
            }
            // Call back to your Kotlin CustomWebView:
            // -> e.g., env->CallVoidMethod(...) to call invalidate() on the View
        }
    }
}