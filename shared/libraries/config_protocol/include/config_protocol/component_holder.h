/*
 * Copyright 2022-2023 Blueberry d.o.o.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once
#include <opendaq/component.h>

BEGIN_NAMESPACE_OPENDAQ

/*#
 * [interfaceLibrary(IComponent, "opendaq")]
 */
DECLARE_OPENDAQ_INTERFACE(IComponentHolder, IBaseObject)
{
    virtual ErrCode INTERFACE_FUNC getLocalId(IString** localId) = 0;
    virtual ErrCode INTERFACE_FUNC getComponent(IComponent** component) = 0;
};

END_NAMESPACE_OPENDAQ