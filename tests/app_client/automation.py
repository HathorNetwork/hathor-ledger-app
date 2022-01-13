import operator
from abc import ABCMeta, abstractmethod
from enum import IntEnum
from functools import reduce
from typing import Tuple
from urllib.parse import urljoin

from requests import Session


class ConditionFlag(IntEnum):
    RUN_ONCE = 1
    RUN_FOR = 2
    NOT_RUN = 3


class Button(IntEnum):
    LEFT = 1
    RIGHT = 2


class Rules:
    """Automation rules and schema are defined in
        https://speculos.ledger.com/user/automation.html
        This class will not implement all functionalities,
        only the ones we need for our tests.

    4 action types are available:
    ["button", num, pressed]    : press (pressed=true) or release (pressed=false)
                                    a button (num=1 for left, num=2 for right)
    ["finger", x, y, touched]   : touch (touched=true) or release (touched=false)
                                    the screen (x and y coordinates)
    ["setbool", varname, value] : set a variable whose name is varname to a boolean
                                    value (either true or false)
    ["exit"]                    : exit speculos

    Obs: The "finger" action only applies to blue which we do not support
    """

    go_right = [
        ["button", Button.RIGHT.value, True],
        ["button", Button.RIGHT.value, False],
    ]

    go_left = [
        ["button", Button.LEFT.value, True],
        ["button", Button.LEFT.value, False],
    ]

    press_both = [
        ["button", Button.LEFT.value, True],
        ["button", Button.RIGHT.value, True],
        ["button", Button.LEFT.value, False],
        ["button", Button.RIGHT.value, False],
    ]

    default_rule = {"actions": [["setbool", "default_match", True]]}

    @classmethod
    def actions(cls, acts):
        return reduce(operator.add, acts)

    @classmethod
    def rule(
        cls,
        actions,
        text: None,
        is_regex: bool = False,
        conditions: Tuple[str, ConditionFlag] = None,
    ) -> dict:
        rule = {}
        if text:
            if is_regex:
                rule["regexp"] = text
            else:
                rule["text"] = text
        if conditions:
            conds = []
            for varname, flag in conditions:
                if flag == ConditionFlag.RUN_FOR:
                    conds.append([varname, True])
                elif flag == ConditionFlag.NOT_RUN:
                    conds.append([varname, False])
                elif flag == ConditionFlag.RUN_ONCE:
                    conds.append([varname, False])
                    actions.append(["setbool", varname, True])
            rule["conditions"] = conds
        rule["actions"] = actions
        return rule

    @classmethod
    def get_address_rule(cls):
        return [
            cls.rule(
                cls.go_left,
                text="Address",
            ),
            cls.rule(
                cls.press_both,
                text="Approve",
            ),
            cls.default_rule,
        ]

    @classmethod
    def get_xpub_rule(cls, confirm: bool):
        if confirm:
            acts = cls.actions([cls.go_left, cls.go_left, cls.press_both])
        else:
            acts = cls.actions([cls.go_left, cls.press_both])
        return [
            cls.rule(
                acts,
                text="access?",
            ),
            cls.default_rule,
        ]

    @classmethod
    def sign_tx_reject_output_rule(cls):
        acts = cls.actions([cls.go_left, cls.press_both])
        return [
            cls.rule(acts, text="Output"),
            cls.default_rule,
        ]

    @classmethod
    def sign_tx_reject_send_rule(cls):
        output_acts = cls.actions([cls.go_left, cls.go_left, cls.press_both])
        send_acts = cls.actions([cls.go_left, cls.press_both])
        return [
            cls.rule(output_acts, text="Output"),
            cls.rule(send_acts, text="Transaction?"),
            cls.default_rule,
        ]

    @classmethod
    def sign_tx_accept_rule(cls):
        output_acts = cls.actions([cls.go_left, cls.go_left, cls.press_both])
        send_acts = cls.actions([cls.go_left, cls.go_left, cls.press_both])
        return [
            cls.rule(output_acts, text="Output"),
            cls.rule(send_acts, text="Transaction?"),
            cls.default_rule,
        ]

    # sign_tx_quit_rule?


class Automation(metaclass=ABCMeta):
    @abstractmethod
    def set_accept_all(self):
        ...

    @abstractmethod
    def close(self):
        ...


class FakeAutomation(Automation):
    def set_accept_all(self):
        pass

    def close(self) -> None:
        pass


class CommandAutomation:
    def __init__(self, server: str) -> None:
        self.server = server
        self.session = Session()

    def close(self) -> None:
        self.session.close()

    def endpoint(self, path: str) -> str:
        return urljoin(self.server, path)

    def automation(self, rules: dict) -> None:
        print("Automation ======")
        print(rules)
        url = self.endpoint("/automation")
        print(url)
        response = self.session.post(
            url,
            json={
                "version": 1,
                "rules": rules,
            },
        )
        print(response.status_code, response.text)
        if response.status_code != 200:
            raise Exception("automation failed")

    def set_accept_all(self):
        rules = [
            Rules.rule(Rules.go_left, "Address"),
            Rules.rule(Rules.go_left, "Output"),
            Rules.rule(Rules.go_left, "Transaction?"),
            Rules.rule(Rules.go_left, "access?"),
            Rules.rule(Rules.go_left, "Confirm token data"),
            Rules.rule(Rules.go_left, "Reset token signatures"),
            Rules.rule(Rules.go_left, "Reject"),
            Rules.rule(Rules.press_both, "Approve"),
        ]
        self.automation(rules)
