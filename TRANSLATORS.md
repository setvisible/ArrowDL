<img align="left" src="./src/resources/logo/icon64.png">

# Translations

## Localization

[Current Status](http://htmlpreview.github.io/?https://github.com/setvisible/DownZemAll/blob/master/.tx/status.html)


## Add a new translation

To add the language 'xx' in country 'XX':

1. Go to Transifex > Language > Add the language

2. Add following line to `./src/src.pro`:
        
        TRANSLATIONS += $$PWD/locale/dza_xx_XX.ts
        
3. Run `lupdate ./src/src.pro -noobsolete`. It creates the *.ts.

4. Copy file `messages.json` from **en** (or **en_US**) to:

        ./web-extension/extension/src/base/_locales/xx_XX/messages.json

5. Add following line to `lang_map` in the Transifex config file `./.tx/config`:

        lang_map = [...], xx_XX: xx_XX

6. That's it. Follow instructions in next sections to update, build and release it.


## Translators Corner

If you are interested in translating this application to another language,
you can use the **Transifex** translation service, or the tool **Qt Linguist**.

### Online Translation

This method requires a **Transifex** user account.

1. Log on your account and go to the [project's page](https://www.transifex.com/downzemall)

2. Edit the translations

Once 100% finished and verified, Transfex pushes the modifications automatically.
A merge into `develop` is however manually required.

(the updated translations should be available for the next release)

Note that unfinished translations may also be deployed from time to time.


### Offline Translation

This method requires:
- [Qt Framework](https://www.qt.io/) (Qt5.14 at the time of writing these lines)
- local clone of the repository

These files are the translations source (*.ts):

    <REPO>/src/locale/dza_en_US.ts
    <REPO>/src/locale/dza_fr_Fr.ts
    ...
    <REPO>/src/locale/dza_zn_CN.ts


Use **Qt Linguist** to edit them.

Then, push the modifications, merge into `develop`,
and follow the classic deployment workflow.

_Rem:_ The advantage of this method over the online method is that the tool
shows the modifications in context within the app's UI, so it's more
confortable to watch what you get and correct the strings, positions,
lengths and other specific problems for a given language.


## Note for Developers

### 1. Qt

Translation files can be desynchronized after an update of the C++ source code.

To resynchronize *.ts with the source code, use `lupdate` and `lrelease`:

    lupdate ./src/src.pro -noobsolete

Then the *.ts files are synchronized with the C++ code.


### 2. (Optional) Transifex

Use the TX Client to push and pull translations to and from Transifex:

> You need `tx` (see Transifex documentation to [install](https://docs.transifex.com/client/installing-the-client) it)


#### Push

When you’re ready to push your updated files to Transifex, use the `tx push` command.
The `-s` flag pushes source files, while the `-t` flag pushes translation files:

    $ tx push -st
    Pushing translations for resource i18n.enjson:
    Pushing source file (src/locale/dza_en_US.ts)
    Pushing 'de_DE' translations (file: src/locale/dza_de_DE.ts)
    Pushing 'es_ES' translations (file: src/locale/dza_es_ES.ts)
    Done.


#### Pull

To pull changes into your local project folder, use `tx pull`:

    $ tx pull -a
    New translations found for the following languages: de, es
    Pulling new translations for resource i18n.enjson (source: src/locale/dza_en_US.ts)
     -> de_DE: src/locale/dza_de_DE.ts
     -> es_ES: src/locale/dza_es_ES.ts
    Done.

At this point, *.ts files are synchronized with Transifex.


### 3. Build .qm from .ts

Before deploying, type:

    lrelease <REPO>/src/src.pro


It generates *.qm (compiled translation files) to be deployed with the application.


## References

- [Internationalization with Qt](https://doc.qt.io/qt-5/internationalization.html)
- [Qt Linguist Manual](https://doc.qt.io/qt-5/qtlinguist-index.html)
- [Transifex Documentation](https://docs.transifex.com/formats/qt-ts)
- [Transifex TX Client](https://docs.transifex.com/transifex-github-integrations/github-tx-client)
