#pragma once
#include "NexusPch.h"
#include <qstringlist.h>

//============================================================================
const QStringList q_order_columns_names = {
    "Order Fill Time","Asset Identifier","Strategy Identifier",
    "Order Type","Units","Average Price","Limit","Order State","Portfolio Identifier",
    "Order Create Time","Order Cancel Time", "Order ID",
};


//============================================================================
const QStringList q_trade_column_names = {
    "Trade Open Time","Trade Close Time","Bars Held","Asset Identifier","Strategy Identifier",
    "Units","Average Price","Close Price","Unrealized PL",
    "Realized PL","Portfolio Identifier","Trade Identifier","Last Price","NLV",
};


//============================================================================
inline std::vector<std::string> qlist_to_str_vec(QStringList const& list) {
    std::vector<std::string> order_columns_names;
    for (const auto& str : list) {
        order_columns_names.push_back(str.toStdString());
    }
    return order_columns_names;
}


//============================================================================
static QIcon svgIcon(const QString& File)
{
    // This is a workaround, because in item views SVG icons are not
    // properly scaled and look blurry or pixelate
    QIcon SvgIcon(File);
    SvgIcon.addPixmap(SvgIcon.pixmap(92));
    return SvgIcon;
}


//============================================================================
inline QStringList str_vec_to_qlist(std::vector<std::string> const& vec) {
	QStringList list;
	for (const auto& str : vec) {
		list.push_back(QString::fromStdString(str));
	}
	return list;
}


//============================================================================
inline std::string json_val_to_string(json const& j)
{
    if (j.is_string()) {
        return j.get<std::string>();
    }
    else if (j.is_number_integer()) {
        return std::to_string(j.get<long long>());
    }
    else if (j.is_number_unsigned()) {
        return std::to_string(j.get<size_t>());
    }
    else if (j.is_number_float()) {
        return std::to_string(j.get<double>());
    }
    else {
        throw std::runtime_error("unexcpeted type");
    }
}
