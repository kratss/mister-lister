#pragma once

#include "types.h"
#include <ftxui/dom/elements.hpp>
#include <map>
#include <string>

using namespace ftxui;

inline const std::map<std::string, Action> key_bindings = {
    // normal
    {"k", Action::move_up},
    {"j", Action::move_down},
    {"h", Action::move_left},
    {"l", Action::move_right},

    {"a", Action::add_entry},
    {"d", Action::remove},
    {"e", Action::edit},
    {"u", Action::undo},
    {"/", Action::search},
    {" ", Action::show_key_map},

    {"q!", Action::quit_without_saving},
    {"ca", Action::column_add},
    {"cd", Action::column_delete},
    {"wq", Action::quit},

    // vim
    {"o", Action::add_entry},
    {"r", Action::edit},
    {"gg", Action::go_to_start},
    {"G", Action::go_to_end},
    {},
    {},
    {},
    {},
    {},
    {},

    // helix
    {"ge", Action::go_to_end},
};

const Element text_normal = vbox({
    text("Default Commands"),
    text(" <--  Alt Command Palettes  -->"),
    text(" hjkl  navigate"),
    text(" wq    save and quit\nasdf"),
    text(" e     edit"),
    text(" a     add row"),
    text(" ca    add column"),
    text(" cd    delete column"),
    text(" d     delete row"),
    text(" /     search"),
    text(" q!    quit without saving"),
    text(" gg    go to top"),
    text(" ge    go to end"),
    text(" u     undo"),
});


const Element text_vim = vbox({
    text("Vim Style Commands"),
    text(" <--  Alt Command Palettes  -->"),
    text(" hjkl  navigate"),
    text(" wq    save and quit\nasdf"),
    text(" e     edit"),
    text(" a     add row"),
    text(" ca    add column"),
    text(" cd    delete column"),
    text(" d     delete row"),
    text(" /     search"),
    text(" q!    quit without saving"),
    text(" gg    go to top"),
    text(" ge    go to end"),
    text(" u     undo"),
});


const Element text_helix = vbox({
    text("Helix Style Commands"),
    text(" <--  Alt Command Palettes  -->"),
    text(" hjkl  navigate"),
    text(" wq    save and quit\nasdf"),
    text(" e     edit"),
    text(" a     add row"),
    text(" ca    add column"),
    text(" cd    delete column"),
    text(" d     delete row"),
    text(" /     search"),
    text(" q!    quit without saving"),
    text(" gg    go to top"),
    text(" ge    go to end"),
    text(" u     undo"),
});

inline const std::vector<Element> key_bindings_text = {
    text_normal,
    text_vim,
    text_helix
};

