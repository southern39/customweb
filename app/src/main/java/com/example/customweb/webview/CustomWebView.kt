package com.example.customweb.webview

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View
import kotlin.math.max

class CustomWebView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyleAttr: Int = 0
) : View(context, attrs, defStyleAttr) {

    companion object {
        init {
            System.loadLibrary("southern_litehtml")
        }
    }

    private var bitmap: Bitmap? = null
    private var htmlContent: String = "<html><body><h1>Loading...</h1></body></html>"
    private var contentWidth = 0 // Width used for rendering HTML
    private var contentHeight = 0 // Height calculated by LiteHTML

    // --- Native Method Declarations ---
    private external fun nativeInit(width: Int, height: Int) // Example
    private external fun nativeRender(html: String, bitmap: Bitmap)
    private external fun nativeGetDocumentHeight(): Int
    private external fun nativeOnTouchEvent(eventType: Int, x: Int, y: Int) // 0:DOWN, 1:UP, 2:MOVE

    fun loadHtml(html: String) {
        this.htmlContent = html
        // Request a remeasure and redraw
        requestLayout() // Important if content size might change
        invalidate()
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        val measuredWidth = MeasureSpec.getSize(widthMeasureSpec)
        // For simplicity, we'll try to render with the measuredWidth.
        // Height will be determined by LiteHTML.

        contentWidth = measuredWidth
        if (contentWidth > 0 && htmlContent.isNotBlank()) {
            // We need a temporary bitmap to get the height from LiteHTML
            // Or, if your nativeRender can work without a bitmap first to just calculate layout.
            // For this example, let's assume nativeGetDocumentHeight can be called
            // after a render pass or it can estimate.
            // A more robust way would be to render to a dummy/small bitmap or have a dedicated layout pass.

            // To get the height, we might need a render pass.
            // This is a bit chicken-and-egg. Let's create a bitmap and render.
            if (bitmap == null || bitmap!!.width != contentWidth ) { // A minimal height for now
                // Recreate bitmap if width changes or it's null
                // Height will be adjusted after first render or by nativeGetDocumentHeight
                bitmap?.recycle()
                // Start with a guessed height, LiteHTML will tell us the actual
                bitmap = Bitmap.createBitmap(contentWidth, 1, Bitmap.Config.ARGB_8888)
            }

            bitmap?.let {
                // This initial render might just be for layout calculation by LiteHTML
                nativeRender(htmlContent, it) // Render to get dimensions
                contentHeight = nativeGetDocumentHeight()
            }
        }

        val finalHeight = if (contentHeight > 0) contentHeight else MeasureSpec.getSize(heightMeasureSpec) // Use calculated or measured
        setMeasuredDimension(measuredWidth, max(finalHeight, minimumHeight)) // Ensure at least minimumHeight
    }


    override fun onSizeChanged(w: Int, h: Int, oldw: Int, oldh: Int) {
        super.onSizeChanged(w, h, oldw, oldh)
        if (w > 0 && h > 0) {
            if (bitmap == null || bitmap!!.width != w || bitmap!!.height != h) {
                bitmap?.recycle()
                // Use the actual view height for the bitmap now if contentHeight wasn't enough
                // or if we want the bitmap to match the view's current drawing area.
                // However, for scrolling content, the bitmap height should be contentHeight.
                // For this example, let's assume the bitmap matches the current view bounds for drawing,
                // but the actual scrolling content is managed by `contentHeight`.
                // This part needs careful thought based on how you handle scrolling.
                // If not scrolling, use h. If scrolling, use contentHeight (from onMeasure).
                val bmHeight = if (contentHeight > h && contentHeight > 0) contentHeight else h
                bitmap = Bitmap.createBitmap(w, bmHeight, Bitmap.Config.ARGB_8888)
            }
            contentWidth = w // Update contentWidth based on actual view size
            // Re-render because size changed
            invalidate()
        } else {
            bitmap?.recycle()
            bitmap = null
        }
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        if (contentWidth <= 0) return // Not measured yet or invalid state

        // Ensure bitmap is ready and matches contentWidth
        // The height of the bitmap should ideally be `contentHeight` if you plan to implement scrolling.
        // For a non-scrolling view, it can be `height`.
        if (bitmap == null || bitmap!!.width != contentWidth || (contentHeight > 0 && bitmap!!.height != contentHeight)) {
            bitmap?.recycle()
            val bmHeight = if (contentHeight > 0) contentHeight else height
            if (contentWidth > 0 && bmHeight > 0) {
                bitmap = Bitmap.createBitmap(contentWidth, bmHeight, Bitmap.Config.ARGB_8888)
            } else {
                return // Cannot create bitmap with zero dimensions
            }
        }


        bitmap?.let { bm ->
            // Clear the bitmap (optional, LiteHTML should draw background)
            // bm.eraseColor(Color.TRANSPARENT) // Or a default background

            // Call native rendering function
            nativeRender(htmlContent, bm)

            // Draw the bitmap onto the canvas
            canvas.drawBitmap(bm, 0f, 0f, null)
        }
    }

    override fun onTouchEvent(event: MotionEvent): Boolean {
        val action = when (event.action) {
            MotionEvent.ACTION_DOWN -> 0
            MotionEvent.ACTION_UP -> 1
            MotionEvent.ACTION_MOVE -> 2
            else -> -1
        }
        if (action != -1) {
            nativeOnTouchEvent(action, event.x.toInt(), event.y.toInt())
            // You might need to call invalidate() if the touch event caused a state change
            // that LiteHTML handled and requires a redraw (e.g., :hover effects).
            // The nativeOnTouchEvent could return a boolean indicating if a redraw is needed.
            invalidate() // For simplicity, always invalidate on touch
            return true // Consume the event
        }
        return super.onTouchEvent(event)
    }

    // Call this when the view is no longer needed to free native resources
    fun release() {
        bitmap?.recycle()
        bitmap = null
        // TODO: Add a nativeRelease() function in C++ to free LiteHTML document, container, etc.
        // nativeRelease()
    }
}