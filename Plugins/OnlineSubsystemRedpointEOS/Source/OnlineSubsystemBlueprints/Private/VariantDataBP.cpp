// Copyright June Rhodes. All Rights Reserved.

#include "VariantDataBP.h"

FVariantDataBP FVariantDataBP::FromNative(const FVariantData &InObj)
{
    FVariantDataBP Result;
    Result.Type = (EOnlineKeyValuePairDataType_)InObj.GetType();
    switch (Result.Type)
    {
    case EOnlineKeyValuePairDataType_::Int32:
        InObj.GetValue(Result.AsInt);
        break;
    case EOnlineKeyValuePairDataType_::Int64:
        InObj.GetValue(Result.AsInt64);
        break;
    case EOnlineKeyValuePairDataType_::String:
        InObj.GetValue(Result.AsString);
        break;
    case EOnlineKeyValuePairDataType_::Float:
        InObj.GetValue(Result.AsFloat);
        break;
    case EOnlineKeyValuePairDataType_::Bool:
        InObj.GetValue(Result.AsBool);
        break;
    case EOnlineKeyValuePairDataType_::Double:
        InObj.GetValue(Result.AsDouble);
        break;
    case EOnlineKeyValuePairDataType_::Empty:
    case EOnlineKeyValuePairDataType_::Json_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt32_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt64_NotSupported:
    case EOnlineKeyValuePairDataType_::Blob_NotSupported:
    default:
        break;
    }
    return Result;
}

FVariantData FVariantDataBP::ToNative() const
{
    FVariantData Result;
    switch (this->Type)
    {
    case EOnlineKeyValuePairDataType_::Int32:
        Result.SetValue(this->AsInt);
        break;
    case EOnlineKeyValuePairDataType_::Int64:
        Result.SetValue(this->AsInt64);
        break;
    case EOnlineKeyValuePairDataType_::String:
        Result.SetValue(this->AsString);
        break;
    case EOnlineKeyValuePairDataType_::Float:
        Result.SetValue(this->AsFloat);
        break;
    case EOnlineKeyValuePairDataType_::Bool:
        Result.SetValue(this->AsBool);
        break;
    case EOnlineKeyValuePairDataType_::Double:
        Result.SetValue(this->AsDouble);
        break;
    case EOnlineKeyValuePairDataType_::Empty:
    case EOnlineKeyValuePairDataType_::Json_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt32_NotSupported:
    case EOnlineKeyValuePairDataType_::UInt64_NotSupported:
    case EOnlineKeyValuePairDataType_::Blob_NotSupported:
    default:
        Result.Empty();
        break;
    }
    return Result;
}