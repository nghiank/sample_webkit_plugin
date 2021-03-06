/*
 IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc. ("Apple") in
 consideration of your agreement to the following terms, and your use, installation, 
 modification or redistribution of this Apple software constitutes acceptance of these 
 terms.  If you do not agree with these terms, please do not use, install, modify or 
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and subject to these 
 terms, Apple grants you a personal, non-exclusive license, under Apple’s copyrights in 
 this original Apple software (the "Apple Software"), to use, reproduce, modify and 
 redistribute the Apple Software, with or without modifications, in source and/or binary 
 forms; provided that if you redistribute the Apple Software in its entirety and without 
 modifications, you must retain this notice and the following text and disclaimers in all 
 such redistributions of the Apple Software.  Neither the name, trademarks, service marks 
 or logos of Apple Computer, Inc. may be used to endorse or promote products derived from 
 the Apple Software without specific prior written permission from Apple. Except as expressly
 stated in this notice, no other rights or licenses, express or implied, are granted by Apple
 herein, including but not limited to any patent rights that may be infringed by your 
 derivative works or by other works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO WARRANTIES, 
 EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS 
 USE AND OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR CONSEQUENTIAL 
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS 
          OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, 
 REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND 
 WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE), STRICT LIABILITY OR 
 OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wtf/Platform.h>
// The buildbot doesn't have Xlib.  Rather than revert this, I've just
// temporarily ifdef'd it out.
#ifdef XLIB_TEMPORARILY_DISABLED
#if PLATFORM(UNIX)
#include <X11/Xlib.h>
#endif
#endif
#include "PluginObject.h"

#ifdef WIN32
#define strcasecmp _stricmp
#define NPAPI WINAPI
#else
#define NPAPI
#endif

// Mach-o entry points
//__declspec(dllexport) NPError NPAPI NP_Initialize(NPNetscapeFuncs *browserFuncs);
//__declspec(dllexport) NPError NPAPI NP_GetEntryPoints(NPPluginFuncs *pluginFuncs);
//__declspec(dllexport) void NPAPI NP_Shutdown(void);

extern "C" {
    NPError NPAPI NP_Initialize(NPNetscapeFuncs *browserFuncs);
    NPError NPAPI NP_GetEntryPoints(NPPluginFuncs *pluginFuncs);
    void NPAPI NP_Shutdown(void);
}


// Mach-o entry points
NPError NPAPI NP_Initialize(NPNetscapeFuncs *browserFuncs)
{
    browser = browserFuncs;
    return NPERR_NO_ERROR;
}

NPError NPAPI NP_GetEntryPoints(NPPluginFuncs *pluginFuncs)
{
    pluginFuncs->version = 11;
    pluginFuncs->size = sizeof(pluginFuncs);
    pluginFuncs->newp = NPP_New;
    pluginFuncs->destroy = NPP_Destroy;
    pluginFuncs->setwindow = NPP_SetWindow;
    pluginFuncs->newstream = NPP_NewStream;
    pluginFuncs->destroystream = NPP_DestroyStream;
    pluginFuncs->asfile = NPP_StreamAsFile;
    pluginFuncs->writeready = NPP_WriteReady;
    pluginFuncs->write = (NPP_WriteProcPtr)NPP_Write;
    pluginFuncs->print = NPP_Print;
    pluginFuncs->event = NPP_HandleEvent;
    pluginFuncs->urlnotify = NPP_URLNotify;
    pluginFuncs->getvalue = NPP_GetValue;
    pluginFuncs->setvalue = NPP_SetValue;
    
    return NPERR_NO_ERROR;
}

void NPAPI NP_Shutdown(void)
{
}

static void executeScript(const PluginObject* obj, const char* script)
{
    NPObject *windowScriptObject;
    browser->getvalue(obj->npp, NPNVWindowNPObject, &windowScriptObject);

    NPString npScript;
    npScript.UTF8Characters = script;
    npScript.UTF8Length = strlen(script);

    NPVariant browserResult;
    browser->evaluate(obj->npp, windowScriptObject, &npScript, &browserResult);
    browser->releasevariantvalue(&browserResult);
}

NPError NPP_New(NPMIMEType pluginType, NPP instance, uint16 mode, int16 argc, char *argn[], char *argv[], NPSavedData *saved)
{
	if (browser->version >= 14) {
		PluginObject* obj = (PluginObject*)browser->createobject(instance, getPluginClass());

		FILE* file = fopen("C:\\fileTest.js", "r");
		char buf[1024];		
		char lazyBuf[1024 * 32];
		size_t nread;
		char *p = &lazyBuf[0];
		while ((nread = fread(buf, 1, sizeof buf, file)) > 0)
		{
			memcpy(p, buf, nread);
			p += nread;
		}
		*p ='\0';
		executeScript(obj, lazyBuf);        
		instance->pdata = obj;
	}
  // On Windows and Unix, plugins only get events if they are windowless.
   return browser->setvalue(instance, NPPVpluginWindowBool, NULL);
}

NPError NPP_Destroy(NPP instance, NPSavedData **save)
{
    PluginObject* obj = static_cast<PluginObject*>(instance->pdata);
    if (obj) {
        if (obj->onStreamLoad)
            free(obj->onStreamLoad);
        
        if (obj->logDestroy)
            printf("PLUGIN: NPP_Destroy\n");

        browser->releaseobject(&obj->header);
    }

    fflush(stdout);

    return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance, NPWindow *window)
{
    if (window->window == NULL) {
        return NPERR_NO_ERROR;
    }
   
    PluginObject* obj = static_cast<PluginObject*>(instance->pdata);

    if (obj) {
        if (obj->logSetWindow) {
            printf("PLUGIN: NPP_SetWindow: %d %d\n", (int)window->width, (int)window->height);
            obj->logSetWindow = false;
        }
    }
    
    return NPERR_NO_ERROR;
}

NPError NPP_NewStream(NPP instance, NPMIMEType type, NPStream *stream, NPBool seekable, uint16 *stype)
{   
    return NPERR_NO_ERROR;
}

NPError NPP_DestroyStream(NPP instance, NPStream *stream, NPReason reason)
{
    return NPERR_NO_ERROR;
}

int32 NPP_WriteReady(NPP instance, NPStream *stream)
{
    return 0;
}

int32 NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
    return 0;
}

void NPP_StreamAsFile(NPP instance, NPStream *stream, const char *fname)
{
}

void NPP_Print(NPP instance, NPPrint *platformPrint)
{
}

int16 NPP_HandleEvent(NPP instance, void *event)
{    
#ifdef WIN32
    // Below is the event handling code.  Per the NPAPI spec, the events don't
    // map directly between operating systems:
    // http://devedge-temp.mozilla.org/library/manuals/2002/plugin/1.0/structures5.html#1000000
    NPEvent* evt = static_cast<NPEvent*>(event);
    short x = static_cast<short>(evt->lParam & 0xffff);
    short y = static_cast<short>(evt->lParam >> 16);
    switch (evt->event) {
        case WM_PAINT:
            printf("PLUGIN: updateEvt\n");
            break;
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
            printf("PLUGIN: mouseDown at (%d, %d)\n", x, y);
            break;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
            printf("PLUGIN: mouseUp at (%d, %d)\n", x, y);
            break;
        case WM_LBUTTONDBLCLK:
        case WM_MBUTTONDBLCLK:
        case WM_RBUTTONDBLCLK:
            break;
        case WM_MOUSEMOVE:
            printf("PLUGIN: adjustCursorEvent\n");
            break;
        case WM_KEYUP:
            // TODO(tc): We need to convert evt->wParam from virtual-key code
            // to key code.
            printf("NOTIMPLEMENTED PLUGIN: keyUp '%c'\n", ' ');
            break;
        case WM_KEYDOWN:
            // TODO(tc): We need to convert evt->wParam from virtual-key code
            // to key code.
            printf("NOTIMPLEMENTED PLUGIN: keyDown '%c'\n", ' ');
            break;
        case WM_SETCURSOR:
            break;
        case WM_SETFOCUS:
            printf("PLUGIN: getFocusEvent\n");
            break;
        case WM_KILLFOCUS:
            printf("PLUGIN: loseFocusEvent\n");
            break;
        default:
            printf("PLUGIN: event %d\n", evt->event);        
    }
    fflush(stdout);
#elif PLATFORM(UNIX)
#ifdef XLIB_TEMPORARILY_DISABLED
    XEvent* evt = static_cast<XEvent*>(event);
    XButtonPressedEvent* bpress_evt = reinterpret_cast<XButtonPressedEvent*>(evt);
    XButtonReleasedEvent* brelease_evt = reinterpret_cast<XButtonReleasedEvent*>(evt);
    switch (evt->type) {
        case ButtonPress:
            printf("PLUGIN: mouseDown at (%d, %d)\n", bpress_evt->x, bpress_evt->y);
            break;
        case ButtonRelease:
            printf("PLUGIN: mouseUp at (%d, %d)\n", brelease_evt->x, brelease_evt->y);
            break;
        case KeyPress:
            // TODO: extract key code
            printf("NOTIMPLEMENTED PLUGIN: keyDown '%c'\n", ' ');
            break;
        case KeyRelease:
            // TODO: extract key code
            printf("NOTIMPLEMENTED PLUGIN: keyUp '%c'\n", ' ');
            break;
        case GraphicsExpose:
            printf("PLUGIN: updateEvt\n");
            break;
        // NPAPI events
        case FocusIn:
            printf("PLUGIN: getFocusEvent\n");
            break;
        case FocusOut:
            printf("PLUGIN: loseFocusEvent\n");
            break;
        case EnterNotify:
        case LeaveNotify:
        case MotionNotify:
            printf("PLUGIN: adjustCursorEvent\n");
            break;
        default:
            printf("PLUGIN: event %d\n", evt->type);
    }

    fflush(stdout);
#endif  // XLIB_TEMPORARILY_DISABLED

#else
    EventRecord* evt = static_cast<EventRecord*>(event);
    Point pt = { evt->where.v, evt->where.h };
    switch (evt->what) {
        case nullEvent:
            // these are delivered non-deterministically, don't log.
            break;
        case mouseDown:
            GlobalToLocal(&pt);
            printf("PLUGIN: mouseDown at (%d, %d)\n", pt.h, pt.v);
            break;
        case mouseUp:
            GlobalToLocal(&pt);
            printf("PLUGIN: mouseUp at (%d, %d)\n", pt.h, pt.v);
            break;
        case keyDown:
            printf("PLUGIN: keyDown '%c'\n", (char)(evt->message & 0xFF));
            break;
        case keyUp:
            printf("PLUGIN: keyUp '%c'\n", (char)(evt->message & 0xFF));
            break;
        case autoKey:
            printf("PLUGIN: autoKey '%c'\n", (char)(evt->message & 0xFF));
            break;
        case updateEvt:
            printf("PLUGIN: updateEvt\n");
            break;
        case diskEvt:
            printf("PLUGIN: diskEvt\n");
            break;
        case activateEvt:
            printf("PLUGIN: activateEvt\n");
            break;
        case osEvt:
            printf("PLUGIN: osEvt - ");
            switch ((evt->message & 0xFF000000) >> 24) {
                case suspendResumeMessage:
                    printf("%s\n", (evt->message & 0x1) ? "resume" : "suspend");
                    break;
                case mouseMovedMessage:
                    printf("mouseMoved\n");
                    break;
                default:
                    printf("%08lX\n", evt->message);
            }
            break;
        case kHighLevelEvent:
            printf("PLUGIN: kHighLevelEvent\n");
            break;
        // NPAPI events
        case getFocusEvent:
            printf("PLUGIN: getFocusEvent\n");
            break;
        case loseFocusEvent:
            printf("PLUGIN: loseFocusEvent\n");
            break;
        case adjustCursorEvent:
            printf("PLUGIN: adjustCursorEvent\n");
            break;
        default:
            printf("PLUGIN: event %d\n", evt->what);
    }
#endif
    
    return 0;
}

void NPP_URLNotify(NPP instance, const char *url, NPReason reason, void *notifyData)
{
    PluginObject* obj = static_cast<PluginObject*>(instance->pdata);
        
    handleCallback(obj, url, reason, notifyData);
}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void *value)
{
    if (variable == NPPVpluginScriptableNPObject) {
        void **v = (void **)value;
        PluginObject* obj = static_cast<PluginObject*>(instance->pdata);
        // Return value is expected to be retained
        browser->retainobject((NPObject *)obj);
        *v = obj;
        return NPERR_NO_ERROR;
    }
    return NPERR_GENERIC_ERROR;
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *value)
{
    return NPERR_GENERIC_ERROR;
}
