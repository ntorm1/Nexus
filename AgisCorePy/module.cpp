#include "pch.h"

#include <pybind11/pybind11.h>
#include <pybind11/functional.h>

#include "AgisStrategy.h"


namespace py = pybind11;
using namespace std;


class PyAgisStrategy : public AgisStrategy {
public:
    /* inherit the constructors */
    using AgisStrategy::AgisStrategy;

    /// <summary>
    /// Trampoline function for pure virtual method: next
    /// </summary>
    void next() override {
        PYBIND11_OVERRIDE_PURE(
            void,
            AgisStrategy,
            next
        );
    }

    /// <summary>
    /// Trampoline function for pure virtual method: reset
    /// </summary>
    void reset() override {
        PYBIND11_OVERRIDE_PURE(
            void,
            AgisStrategy,
            reset
        );
    }

    /// <summary>
    /// Trampoline function for pure virtual method: build
    /// </summary>
    void build() override {
        PYBIND11_OVERRIDE_PURE(
            void,
            AgisStrategy,
            build
        );
    }
};


void init_agis_strategy(py::module& m)
{
    py::enum_<AllocType>(m, "AllocType")
        .value("UNITS", AllocType::UNITS)
        .value("DOLLARS", AllocType::DOLLARS)
        .value("PCT", AllocType::PCT)
        .export_values();

    py::class_<AgisStrategy, PyAgisStrategy>(m, "AgisStrategy")
        .def(py::init<const std::string&, std::shared_ptr<Portfolio>, double>())
        .def("next", &AgisStrategy::next)
        .def("reset", &AgisStrategy::reset)
        .def("build", &AgisStrategy::build)
        .def("get_nlv", &AgisStrategy::get_nlv, "Get net liquidation value")
        .def("exchange_subscribe", &AgisStrategy::exchange_subscribe, "Subscribe to an exchange")
        .def("get_allocation", &AgisStrategy::get_allocation, "Get portfolio allocation")
        .def("get_strategy_index", &AgisStrategy::get_strategy_index, "Get unique strategy index")
        .def("get_strategy_id", &AgisStrategy::get_strategy_id, "Get unique strategy id")
        .def("get_portfolio_index", &AgisStrategy::get_portfolio_index, "Get portfolio index")
        .def("get_portfolio_id", &AgisStrategy::get_portfolio_id, "Get portfolio id")
        .def("get_exchange", &AgisStrategy::get_exchange, "Get an exchange by id");
}

void init_exchange(py::module& m)
{
    py::enum_<ExchangeQueryType>(m, "ExchangeQueryType")
        .value("Default", ExchangeQueryType::Default)
        .value("NExtreme", ExchangeQueryType::NExtreme)
        .value("NLargest", ExchangeQueryType::NLargest)
        .value("NSmallest", ExchangeQueryType::NSmallest)
        .export_values();

    py::class_<Exchange, std::shared_ptr<Exchange>>(m, "Exchange")
        .def("get_exchange_view",
            py::overload_cast<
            const std::string&,
            int,
            ExchangeQueryType,
            int,
            bool>
        (&Exchange::get_exchange_view),
        "Get ExchangeView with specified parameters");

    py::class_<ExchangeView>(m, "ExchangeView")
        .def(py::init<>())
        .def_readwrite("view", &ExchangeView::view)
        .def_readwrite("exchange_index", &ExchangeView::exchange_index)
        .def("__sub__", &ExchangeView::operator-, py::is_operator())  // Wrap the subtraction operator
        .def("__add__", &ExchangeView::operator+, py::is_operator())  // Wrap the subtraction operator
        .def("__div__", &ExchangeView::operator/, py::is_operator())  // Wrap the subtraction operator
        .def("__mul__", &ExchangeView::operator*, py::is_operator());  // Wrap the subtraction operator
    
}

void init_portfolio(py::module& m)
{
    py::class_<Portfolio, std::shared_ptr<Portfolio>>(m, "Portfolio")
        .def(py::init<const std::string&, double>());
}

PYBIND11_MODULE(AgisCorePy, m) {
    // build python bindings for Agis strategy class
    init_agis_strategy(m);

    // build python bindings for exchange class
    init_exchange(m);

    // build python bindings for portfolio class
    init_portfolio(m);
}