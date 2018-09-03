// Copyright (c) 2018 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// ---------------------------------------------------------------------------
//
// This file was generated by the CEF translator tool. If making changes by
// hand only do so within the body of existing method and function
// implementations. See the translator.README.txt file in the tools directory
// for more information.
//
// $hash=c00ed48771c61f0701fd439ddc7c270f7d7601b8$
//

#include "libcef_dll/ctocpp/extension_ctocpp.h"
#include "libcef_dll/cpptoc/extension_handler_cpptoc.h"
#include "libcef_dll/ctocpp/dictionary_value_ctocpp.h"
#include "libcef_dll/ctocpp/request_context_ctocpp.h"

// VIRTUAL METHODS - Body may be edited by hand.

CefString CefExtensionCToCpp::GetIdentifier() {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_identifier))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_identifier(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

CefString CefExtensionCToCpp::GetPath() {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_path))
    return CefString();

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_string_userfree_t _retval = _struct->get_path(_struct);

  // Return type: string
  CefString _retvalStr;
  _retvalStr.AttachToUserFree(_retval);
  return _retvalStr;
}

CefRefPtr<CefDictionaryValue> CefExtensionCToCpp::GetManifest() {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_manifest))
    return NULL;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_dictionary_value_t* _retval = _struct->get_manifest(_struct);

  // Return type: refptr_same
  return CefDictionaryValueCToCpp::Wrap(_retval);
}

bool CefExtensionCToCpp::IsSame(CefRefPtr<CefExtension> that) {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_same))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Verify param: that; type: refptr_same
  DCHECK(that.get());
  if (!that.get())
    return false;

  // Execute
  int _retval = _struct->is_same(_struct, CefExtensionCToCpp::Unwrap(that));

  // Return type: bool
  return _retval ? true : false;
}

CefRefPtr<CefExtensionHandler> CefExtensionCToCpp::GetHandler() {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_handler))
    return NULL;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_extension_handler_t* _retval = _struct->get_handler(_struct);

  // Return type: refptr_diff
  return CefExtensionHandlerCppToC::Unwrap(_retval);
}

CefRefPtr<CefRequestContext> CefExtensionCToCpp::GetLoaderContext() {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, get_loader_context))
    return NULL;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  cef_request_context_t* _retval = _struct->get_loader_context(_struct);

  // Return type: refptr_same
  return CefRequestContextCToCpp::Wrap(_retval);
}

bool CefExtensionCToCpp::IsLoaded() {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, is_loaded))
    return false;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  int _retval = _struct->is_loaded(_struct);

  // Return type: bool
  return _retval ? true : false;
}

void CefExtensionCToCpp::Unload() {
  cef_extension_t* _struct = GetStruct();
  if (CEF_MEMBER_MISSING(_struct, unload))
    return;

  // AUTO-GENERATED CONTENT - DELETE THIS COMMENT BEFORE MODIFYING

  // Execute
  _struct->unload(_struct);
}

// CONSTRUCTOR - Do not edit by hand.

CefExtensionCToCpp::CefExtensionCToCpp() {}

template <>
cef_extension_t*
CefCToCppRefCounted<CefExtensionCToCpp, CefExtension, cef_extension_t>::
    UnwrapDerived(CefWrapperType type, CefExtension* c) {
  NOTREACHED() << "Unexpected class type: " << type;
  return NULL;
}

#if DCHECK_IS_ON()
template <>
base::AtomicRefCount
    CefCToCppRefCounted<CefExtensionCToCpp, CefExtension, cef_extension_t>::
        DebugObjCt ATOMIC_DECLARATION;
#endif

template <>
CefWrapperType CefCToCppRefCounted<CefExtensionCToCpp,
                                   CefExtension,
                                   cef_extension_t>::kWrapperType =
    WT_EXTENSION;
