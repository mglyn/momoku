#ifndef ENGINE
#define ENGINE

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "position.h"
#include "search.h"
#include "tt.h"

struct OptionsMap;

struct Option {

    std::string       defaultValue, currentValue, type;
    int               min, max;
    size_t            idx;
    const OptionsMap* optionsMap = nullptr;

    void operator<<(const Option&);

public:
    Option();
    Option(bool v);
    Option(const char* v);
    Option(double v, int minv, int maxv);
    Option(const char* v, const char* curr);

    Option& operator=(const std::string&);
    operator int() const;
    operator std::string() const;
    bool operator==(const char*) const;
    bool operator!=(const char*) const;

    friend std::ostream& operator<<(std::ostream&, const OptionsMap&);
};
class OptionsMap {

};



class Engine {

    int gameSize;
    Position                pos;
    std::vector<StateInfo>  stList;
    std::vector<Sqare>      PV;
    std::map<std::string, bool> options;
    CommandLine
    Search::SearchManager::UpdateContext  updateContext;
    std::function<void(std::string_view)> onVerifyNetworks;
public:

    Engine(std::optional<std::string> path = std::nullopt);

    // Cannot be movable due to components holding backreferences to fields
    Engine(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine& operator=(Engine&&) = delete;

    ~Engine() { wait_for_search_finished(); }

    std::uint64_t perft(const std::string& fen, Depth depth);

    // non blocking call to start searching
    void go(Search::LimitsType&);
    // non blocking call to stop searching
    void stop();

    // blocking call to wait for search to finish
    void wait_for_search_finished();
    // set a new position, moves are in UCI format
    void set_position(const std::string& fen, const std::vector<std::string>& moves);

    // modifiers

    void set_numa_config_from_option(const std::string& o);
    void resize_threads();
    void set_tt_size(size_t mb);
    void set_ponderhit(bool);
    void search_clear();

    void set_on_update_no_moves(std::function<void(const InfoShort&)>&&);
    void set_on_update_full(std::function<void(const InfoFull&)>&&);
    void set_on_iter(std::function<void(const InfoIter&)>&&);
    void set_on_bestmove(std::function<void(std::string_view, std::string_view)>&&);
    void set_on_verify_networks(std::function<void(std::string_view)>&&);

    // network related

    void verify_network() const;
    void load_network(const std::string& file);
    void save_network(const std::optional<std::string>& file);

    // utility functions

    void trace_eval() const;

    const OptionsMap& get_options() const;
    OptionsMap& get_options();

    int get_hashfull(int maxAge = 0) const;

    std::string                            fen() const;
    void                                   flip();
    std::string                            visualize() const;
    std::vector<std::pair<size_t, size_t>> get_bound_thread_count_by_numa_node() const;
    std::string                            get_numa_config_as_string() const;
    std::string                            numa_config_information_as_string() const;
    std::string                            thread_allocation_information_as_string() const;
    std::string                            thread_binding_information_as_string() const;
};
#endif 