/*!
 * BlueprintToRSTDoc
 *
 * Copyright (c) 2020-2023 nutti
 *
 * This software is released under the MIT License.
 * https://opensource.org/licenses/MIT
 */

#pragma once

#define ERROR_MESSAGE_BOX(Message)                                                                                 \
	FPlatformMisc::MessageBoxExt(EAppMsgType::Ok,                                                                  \
		*FString::Format(TEXT("Error:\n\n{0}\n\n---\n\n{1} (Line:{2})"), {Message, *FString(__FILE__), __LINE__}), \
		TEXT("BlueprintToRSTDoc"));