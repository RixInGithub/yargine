== GENERAL ==
note: every gql request needs `X-Auth-Token` from `~/.castle/config.json` + optionally `X-OS` (or `X-Platform`), `X-Scene-Creator-Version` + some headers to accept/post json.

to parse json, use https://github.com/open-source-patterns/nanojsonc
== NEW DECKS ==
no create deck mutation was found, however you can use the `updateCardAndDeckV2` mutation to create the deck and the classic cli update to add to the card

query:
```gql
mutation($a:DeckInput!,$b:CardInput!){updateCardAndDeckV2(deck:$a,card:$b){deck{deckId}card{cardId}}}
```
vars:
```json
{"a":{"deckId":"%s"},"b":{"cardId":"%s","deckId":"%s","blocks":[]}}
```

format with `deckId, cardId, deckId`
== PALETTES ==
i found this:
```js
            r4 = {'key': 'default', 'label': 'AAP-64-Castle', 'creator': 'AdigunPolack', 'isEnabled': true};
            r3 = ['#3b1725', '#73172d', '#b4202a', '#df3e23', '#fa6a0a', '#f9a31b', '#ffd541', '#fffc40', '#fffd98', '#d6f264', '#9cdb43', '#59c135', '#14a02e', '#1a7a3e', '#24523b', '#122020', '#143464', '#285cc4', '#249fde', '#20d6c7', '#a6fcdb', '#fff9db', '#fef3c0', '#fad6b8', '#f5a097', '#f2838b', '#e86a73', '#bc4a9b', '#793a80', '#403353', '#242234', '#322b28', '#71413b', '#bb7547', '#dba463', '#f4d29c', '#edeff2', '#dae0ea', '#b3b9d1', '#8b93af', '#6d758d', '#4a5462', '#333941', '#422433', '#5b3138', '#8e5252', '#ba756a', '#e9b5a3', '#e3e6ff', '#b9bffb', '#849be4', '#588dbe', '#477d85', '#23674e', '#328464', '#5daf8d', '#92dcba', '#cdf7e2', '#e4d2aa', '#c7b08b', '#a08662', '#796755', '#5a4e44', '#423934'];
            r4['colors'] = r3;
            r3 = new Array(2);
```

from the react native decomp, which is neat because the palettes query doesnt include this in for some reason.

query:
```gql
{palettes{colors,isEnabled}}
```
crunched up for efficiency