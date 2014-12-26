/*
 * (C) 2009 - 2015 Benjamin Klein, Dirk Hoffmann. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "C64GUI.h"

using std::make_pair;

// ---------------------------------------------------------------------------------------------
//                                             JoystickManager
// ---------------------------------------------------------------------------------------------

const int JoystickManager::UsageToSearch[][ 2 ] = 
{
	/* Page */						/* Usage (0 for none ) */
	{	kHIDPage_GenericDesktop,	kHIDUsage_GD_Joystick	},
	{	kHIDPage_GenericDesktop,	kHIDUsage_GD_GamePad	},
};
const unsigned JoystickManager::MaxJoystickCount = 2;

JoystickManager::JoystickManager( C64Proxy *proxy )
{
    _proxy = proxy;
    _initialized = false;
    _manager = NULL;
    locationID1 = 0;
    locationID2 = 0;
    proxy1 = NULL;
    proxy2 = NULL;
}

JoystickManager::~JoystickManager()
{
    Dispose();
}

void JoystickManager::addJoystickProxyWithLocationID(int locationID, JoystickProxy *proxy)
{
    if (proxy1 == NULL) {
        locationID1 = locationID;
        proxy1 = proxy;
        return;
    }
    if (proxy1 == NULL) {
        locationID2 = locationID;
        proxy2 = proxy;
        return;
    }
}

JoystickProxy *JoystickManager::getJoystickProxyWithLocationID(int locationID)
{
    if (locationID1 == locationID) {
        assert(proxy1 != NULL);
        return proxy1;
    }
    if (locationID2 == locationID) {
        assert(proxy2 != NULL);
        return proxy2;
    }
    return NULL;
}

void JoystickManager::removeJoystickProxyWithLocationID(int id)
{
    if (locationID1 == id) {
        locationID1 = 0;
        proxy1 = NULL;
        return;
    }
    if (locationID2 == id) {
        locationID2 = 0;
        proxy2 = NULL;
        return;
    }
}

bool JoystickManager::Initialize()
{
    NSLog(@"JostickManager::Initialize");
    
	CFMutableArrayRef matchingArray = CFArrayCreateMutable( kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks );
	if( !matchingArray )
	{
		NSLog( @"%s: out of memory\n", __PRETTY_FUNCTION__ );
		return false;
	}
	for( unsigned n = 0; n < sizeof( UsageToSearch ) / sizeof( UsageToSearch[ 0 ] ); n++)
	{
		CFMutableDictionaryRef dict = CFDictionaryCreateMutable( kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks );
		if( !dict )
		{
			NSLog( @"%s: out of memory\n", __PRETTY_FUNCTION__ );
			
			CFRelease( matchingArray );
			return false;
		}
		
		CFNumberRef number = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &UsageToSearch[ n ][ 0 ] );
		if( !number )
		{
			NSLog( @"%s: out of memory\n", __PRETTY_FUNCTION__ );
			
			CFRelease( matchingArray );
			CFRelease( dict );
			return false;	
		}
		CFDictionarySetValue( dict, CFSTR( kIOHIDDeviceUsagePageKey ), number );
		CFRelease( number );
		
		if( UsageToSearch[ n ][ 1 ] )
		{
			number = CFNumberCreate( kCFAllocatorDefault, kCFNumberIntType, &UsageToSearch[ n ][ 1 ] );
			if( !number )
			{
				NSLog( @"%s: out of memory\n", __PRETTY_FUNCTION__ );
				
				CFRelease( matchingArray );
				CFRelease( dict );
				return false;	
			}
			CFDictionarySetValue( dict, CFSTR( kIOHIDDeviceUsageKey ), number );
			CFRelease( number );
		}
		
		CFArrayAppendValue( matchingArray, dict );
		CFRelease( dict );
	}
	
	_manager = IOHIDManagerCreate( kCFAllocatorDefault, 0 );
	if( !_manager )
	{
		NSLog( @"%s: out of memory\n", __PRETTY_FUNCTION__ );
		CFRelease( matchingArray );
		return false;
	}
	
	IOHIDManagerSetDeviceMatchingMultiple( _manager, matchingArray );
	CFRelease( matchingArray );
	IOHIDManagerRegisterDeviceMatchingCallback( _manager, MatchingCallback_static, this );
	
	IOHIDManagerScheduleWithRunLoop( _manager, CFRunLoopGetMain(), kCFRunLoopDefaultMode );
	
	IOReturn status;
	if( ( status = IOHIDManagerOpen( _manager, kIOHIDOptionsTypeNone ) ) == kIOReturnSuccess )
	{
		_initialized = true;
		return true;
	}
	else
	{
		NSLog( @"%s: failed to open manager (status=%i)\n", __PRETTY_FUNCTION__, status );
		return false;
	}
}

void JoystickManager::Dispose()
{ 
	IOHIDManagerClose( _manager, kIOHIDOptionsTypeNone );
	CFRelease( _manager );
}

void 
JoystickManager::MatchingCallback_static( void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef )
{ 
    ( (JoystickManager *) inContext)->MatchingCallback( inContext, inResult, inSender, inIOHIDDeviceRef );
}

void 
JoystickManager::MatchingCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDDeviceRef inIOHIDDeviceRef)
{
    NSLog(@"MatchingCallback:inIOHIDDeviceRef = %@", inIOHIDDeviceRef);
    
	IOHIDDeviceInfo devInfo = IOHIDDeviceInfo(inIOHIDDeviceRef);
	
	if( inResult != kIOReturnSuccess )
	{
		NSLog( @"%s: device %p (%s) is not in successful state (%i)\n", __PRETTY_FUNCTION__, inIOHIDDeviceRef, devInfo.GetName(), inResult );
		return;
	}
	
    if(getJoystickProxyWithLocationID(devInfo.GetLocationID()) != NULL) {
        NSLog(@"%s: device %p (%s) already opend.\n",
              __PRETTY_FUNCTION__, inIOHIDDeviceRef, devInfo.GetName() );
        return;
	}
	
    if(proxy1 != NULL && proxy2 != NULL) {
		NSLog(@"%s: device %p (%s) will not be opend -> maximum device count reached.\n",
              __PRETTY_FUNCTION__, inIOHIDDeviceRef, devInfo.GetName());
		return;
	}
	
	IOReturn status;
	if( ( status = IOHIDDeviceOpen(inIOHIDDeviceRef, kIOHIDOptionsTypeNone ) ) != kIOReturnSuccess) {
		NSLog(@"%s: failed to open device %p (%s) (status=%i)\n",
              __PRETTY_FUNCTION__, inIOHIDDeviceRef, devInfo.GetName(), status );
		return;
	}
	
    // Prepare context information (will be passed to the callback functions)
    CallbackContext *context = (CallbackContext *)malloc(sizeof(CallbackContext));
    context->manager = this;
    context->locationID = devInfo.GetLocationID();
    context->deviceRef = inIOHIDDeviceRef;
    
    // Register call back functions
	IOHIDDeviceRegisterRemovalCallback( inIOHIDDeviceRef, RemoveCallback_static, (void *)context);
	IOHIDDeviceRegisterInputValueCallback( inIOHIDDeviceRef, InputValueCallback_static, (void *)context); 
	
	Joystick *joystick = [_proxy addJoystick ];
	if( !joystick )
	{
		IOHIDDeviceRegisterInputValueCallback( inIOHIDDeviceRef, NULL, NULL );
		IOHIDDeviceRegisterRemovalCallback( inIOHIDDeviceRef, NULL, NULL );
		IOHIDDeviceClose( inIOHIDDeviceRef, kIOHIDOptionsTypeNone );
		
		NSLog(@"%s: joystick coulnd't be created for %p (%s)\n",
              __PRETTY_FUNCTION__, inIOHIDDeviceRef, devInfo.GetName());
		return;
	}
	
    JoystickProxy *proxy = new JoystickProxy(joystick);
    addJoystickProxyWithLocationID(devInfo.GetLocationID(), proxy);
    
	NSLog(@"%s: opened device %p (%s) %d %x\n", __PRETTY_FUNCTION__,
          inIOHIDDeviceRef, devInfo.GetName(), devInfo.GetLocationID(), devInfo.GetLocationID());
}

void 
JoystickManager::RemoveCallback_static( void *inContext, IOReturn inResult, void *inSender )
{
    assert (inContext != NULL);
    ( (JoystickManager *) inContext)->RemoveCallback( inContext, inResult, inSender ); 
}

void 
JoystickManager::RemoveCallback( void *inContext, IOReturn inResult, void *inSender )
{
    NSLog(@"JoystickManager::RemoveCallback");
    
	IOHIDDeviceInfo devInfo = IOHIDDeviceInfo( (IOHIDDeviceRef) inSender );
	    
	if(inResult != kIOReturnSuccess) {
		NSLog( @"%s: device %p (%s) is not in successful state (%i)\n",
              __PRETTY_FUNCTION__, inSender, devInfo.GetName(), inResult );
		return;
	}

    NSLog(@"Searching for %s: device %p (%s) in open list\n", __PRETTY_FUNCTION__, inSender, devInfo.GetName() );
    
    JoystickProxy *proxy = getJoystickProxyWithLocationID(devInfo.GetLocationID());
    if(proxy == NULL) {
		NSLog(@"%s: device %p (%s) not found in open list\n",
              __PRETTY_FUNCTION__, inSender, devInfo.GetName() );
		return;
	}
	
	IOHIDDeviceRegisterInputValueCallback((IOHIDDeviceRef)inSender, NULL, this);
	IOHIDDeviceRegisterRemovalCallback((IOHIDDeviceRef)inSender, NULL, this);
	IOHIDDeviceClose((IOHIDDeviceRef)inSender, kIOHIDOptionsTypeNone);
	
    Joystick *joystick = proxy->GetJoystick();
    [_proxy removeJoystick:joystick];
    removeJoystickProxyWithLocationID(devInfo.GetLocationID());

    NSLog(@"%s: closed device %p (%s)\n",
          __PRETTY_FUNCTION__, inSender, devInfo.GetName());
}

void 
JoystickManager::InputValueCallback_static( void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef )
{ 
    CallbackContext *context = (CallbackContext *)inContext;
    context->manager->InputValueCallback(inContext, inResult, inSender, inIOHIDValueRef); 
}

void 
JoystickManager::InputValueCallback(void *inContext, IOReturn inResult, void *inSender, IOHIDValueRef inIOHIDValueRef )
{
    CallbackContext *context = (CallbackContext *)inContext;
            
	if( inResult != kIOReturnSuccess )
	{
		NSLog( @"%s: device %p is not in successful state (%i)\n", __PRETTY_FUNCTION__, context->deviceRef, inResult );
		return;
	}
	
	map<int, JoystickProxy>::iterator it;
    JoystickProxy *proxy = getJoystickProxyWithLocationID(context->locationID);
    if (proxy == NULL) {
		NSLog( @"%s: device %p not found in open list\n",
              __PRETTY_FUNCTION__, context->deviceRef);
		return;
	}
    // JoystickProxy &joystick = *proxy;
    
	IOHIDElementRef element = IOHIDValueGetElement( inIOHIDValueRef );
	IOHIDElementType elementType = IOHIDElementGetType( element );
	uint32_t elementPage = IOHIDElementGetUsagePage( element );
	uint32_t elementUsage = IOHIDElementGetUsage( element );
	
	if( elementType == kIOHIDElementTypeInput_Button )
	{
		if( elementPage == kHIDPage_Button )
		{
			// set values to conform to 0 and 1
			IOHIDElement_SetDoubleProperty( element, CFSTR( kIOHIDElementCalibrationMinKey ), 0 );
			IOHIDElement_SetDoubleProperty( element, CFSTR( kIOHIDElementCalibrationMaxKey ), 1 );
			IOHIDElement_SetDoubleProperty( element, CFSTR( kIOHIDElementCalibrationGranularityKey ), 1 );
			bool pressed = ( ceil( IOHIDValueGetScaledValue( inIOHIDValueRef, kIOHIDValueScaleTypeCalibrated ) ) == 1 );
			
			proxy->ChangeButton(elementUsage, pressed);
		}
		else
			NSLog( @"%s: device %p type and page mismatch (Type=%i, Page=%i)\n", 
                  __PRETTY_FUNCTION__, context->deviceRef, elementType, elementPage );
		
	}
	else if( ( elementType == kIOHIDElementTypeInput_Axis ) || ( elementType == kIOHIDElementTypeInput_Misc /* why misc? */ ) )
	{
		if( elementPage == kHIDPage_GenericDesktop )
		{
			// set values to conform to -1 / 0 / 1
			IOHIDElement_SetDoubleProperty( element, CFSTR( kIOHIDElementCalibrationMinKey ), -1 );
			IOHIDElement_SetDoubleProperty( element, CFSTR( kIOHIDElementCalibrationMaxKey ), 1 );
			IOHIDElement_SetDoubleProperty( element, CFSTR( kIOHIDElementCalibrationGranularityKey ), 1 );
			int axis = ceil( IOHIDValueGetScaledValue( inIOHIDValueRef, kIOHIDValueScaleTypeCalibrated ) );
			
			switch( elementUsage )
			{
				case kHIDUsage_GD_X:
					if( axis == -1 )
						proxy->ChangeAxisX(JOYSTICK_AXIS_X_LEFT);
					else if( axis == 1 )
						proxy->ChangeAxisX(JOYSTICK_AXIS_X_RIGHT);
					else
						proxy->ChangeAxisX(JOYSTICK_AXIS_X_NONE);
					break;
				case kHIDUsage_GD_Y:
					if( axis == -1 )
						proxy->ChangeAxisY(JOYSTICK_AXIS_Y_UP );
					else if( axis == 1 )
						proxy->ChangeAxisY(JOYSTICK_AXIS_Y_DOWN);
					else
						proxy->ChangeAxisY(JOYSTICK_AXIS_Y_NONE);
					break;
				default:
					NSLog(@"%s: device %p page and page usage mismatch (Type=%i, Page=%i)\n",
                          __PRETTY_FUNCTION__, context->deviceRef, elementPage, elementUsage);
			}
		}
		else
			NSLog(@"%s: device %p type and page mismatch (Type=%i, Page=%i)\n", 
                  __PRETTY_FUNCTION__, context->deviceRef, elementType, elementPage);
	}
}

void 
JoystickManager::IOHIDElement_SetDoubleProperty( IOHIDElementRef element, CFStringRef key, double value )
{
	CFNumberRef number = CFNumberCreate( kCFAllocatorDefault, kCFNumberDoubleType, &value );
    if ( number ) 
	{
        IOHIDElementSetProperty( element, key, number );
        CFRelease( number );
    }
}

// ---------------------------------------------------------------------------------------------
//                                             JoystickProxy
// ---------------------------------------------------------------------------------------------

JoystickProxy::JoystickProxy()
{
    _joystick = NULL;
    
}

JoystickProxy::JoystickProxy(Joystick *joystick)
{
    _joystick = joystick;
}

void JoystickProxy::ChangeButton( int index, bool pressed )
{
	bool found = ( _pressedButtons.find( index ) != _pressedButtons.end() );
	if( pressed )
	{
		if( !found )
			_pressedButtons.insert( index );
	}
	else
	{
		if ( found )
			_pressedButtons.erase( index );
	}
	
	_joystick->SetButtonPressed( ( _pressedButtons.size() != 0 ) );
}
void JoystickProxy::ChangeAxisX( JoystickAxisState state ) const
{ _joystick->SetAxisX( state ); }
void JoystickProxy::ChangeAxisY( JoystickAxisState state ) const
{ _joystick->SetAxisY( state ); }
Joystick *JoystickProxy::GetJoystick() const
{ return _joystick; }

// ---------------------------------------------------------------------------------------------
//                                             IOHIDDeviceInfo
// ---------------------------------------------------------------------------------------------

IOHIDDeviceInfo::IOHIDDeviceInfo()
{
    _name = NULL;
    _locationID = 0;
}

IOHIDDeviceInfo::IOHIDDeviceInfo( const IOHIDDeviceInfo &copy )
{
	if( copy._name )
	{
		size_t len = strlen( copy._name );
		_name = new char[ len + 1 ];
		strncpy( _name, copy._name, len );
	}
	
	_locationID = copy._locationID;
}

IOHIDDeviceInfo::IOHIDDeviceInfo( IOHIDDeviceRef device )
{
	CFTypeRef typeRef;

    _name = NULL;
    _locationID = 0;

    if (device == NULL) {
        NSLog(@"IOHIDDeviceRef == NULL");
        return;
    }
             
	if( (typeRef = IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey)))) {

        if (CFStringGetTypeID() == CFGetTypeID(typeRef)) {
            CFIndex len = CFStringGetLength( (CFStringRef) typeRef ) + 1;
		
            _name = new char[ len ];
            _name[ len - 1 ] = 0;
		
            CFStringGetCString( (CFStringRef) typeRef, _name, len, kCFStringEncodingMacRoman );
        }
    }
	
	if( ( typeRef = IOHIDDeviceGetProperty( device, CFSTR( kIOHIDLocationIDKey ) ) )  &&  ( CFNumberGetTypeID() == CFGetTypeID( typeRef ) ) )
		CFNumberGetValue( (CFNumberRef) typeRef, kCFNumberIntType, &_locationID );	
}

IOHIDDeviceInfo::~IOHIDDeviceInfo()
{
	if( _name )
		delete [] _name;
}
