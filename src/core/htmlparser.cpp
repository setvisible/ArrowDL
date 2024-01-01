/* - ArrowDL - Copyright (C) 2019-present Sebastien Vavassori
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; If not, see <http://www.gnu.org/licenses/>.
 */

#include "htmlparser.h"

#include <Core/Model>
#include <Core/ResourceItem>
#include <Core/ResourceModel>

#include "gumbo.h"
#include "error.h"

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QRegularExpression>


static ResourceItem* createResourceItem(const GumboElement &element, const QUrl &baseUrl)
{
    auto attributes = &element.attributes;

    GumboAttribute* href = nullptr;
    GumboAttribute* alt = nullptr;
    GumboAttribute* title = nullptr;

    if (element.tag == GUMBO_TAG_A) {
        href = gumbo_get_attribute(attributes, "href");
        alt = gumbo_get_attribute(attributes, "alt");
        title = gumbo_get_attribute(attributes, "title");

    } else if (element.tag == GUMBO_TAG_IMAGE ||
               element.tag == GUMBO_TAG_IMG) {
        href = gumbo_get_attribute(attributes, "src");
        alt = gumbo_get_attribute(attributes, "alt");
        title = gumbo_get_attribute(attributes, "title");

        /// \todo GUMBO_TAG_IMAGE
        /// \todo GUMBO_TAG_IMG
        /// \todo GUMBO_TAG_IFRAME
        /// \todo GUMBO_TAG_EMBED
        /// \todo GUMBO_TAG_OBJECT
        /// \todo GUMBO_TAG_PARAM
        /// \todo GUMBO_TAG_VIDEO
        /// \todo GUMBO_TAG_AUDIO
        /// \todo GUMBO_TAG_SOURCE
    }

    if (href == nullptr) {
        return nullptr;
    }

    QUrl url2(href->value);
    if (url2.isEmpty()) {
        return nullptr;
    }
    auto url3 = baseUrl.resolved(url2);

    auto url = url3.toString();

    auto fullfilename = url;

    {
        if (!fullfilename.isEmpty()) {
            static QRegularExpression re{
                "^(.*)(" + QRegularExpression::escape("/") + ")$",
                        QRegularExpression::CaseInsensitiveOption};
            fullfilename = fullfilename.replace(re, "\\1");
        }
        fullfilename.replace("http://", "", Qt::CaseInsensitive);
        fullfilename.replace("https://", "", Qt::CaseInsensitive);
        fullfilename.replace("#", "_", Qt::CaseInsensitive);
        fullfilename.replace("?", "_", Qt::CaseInsensitive);
        fullfilename.replace("%20", "_", Qt::CaseInsensitive);
        fullfilename.replace("&", "_", Qt::CaseInsensitive);

        fullfilename = fullfilename.trimmed();
        fullfilename = QDir::toNativeSeparators(fullfilename);
    }

    QString titles = title ? QString(title->value) : QString();
    QString alts = alt ? QString(alt->value) : QString();

    auto description = !alts.isEmpty() ? alts : titles;

    auto item = new ResourceItem();
    item->setUrl(url);
    item->setDescription(description);
    return item;
}

static void searchForLinks(GumboNode* node, Model *model, const QUrl &url)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    if (node->v.element.tag == GUMBO_TAG_A) {

        auto item = createResourceItem(node->v.element, url);
        if (item) {
            auto linkModel = model->linkModel();
            linkModel->add(item);
        }

    } else if (node->v.element.tag == GUMBO_TAG_IMAGE ||
               node->v.element.tag == GUMBO_TAG_IMG) {

        /// \todo GUMBO_TAG_IMAGE
        /// \todo GUMBO_TAG_IMG
        /// \todo GUMBO_TAG_IFRAME
        /// \todo GUMBO_TAG_EMBED
        /// \todo GUMBO_TAG_OBJECT
        /// \todo GUMBO_TAG_PARAM
        /// \todo GUMBO_TAG_VIDEO
        /// \todo GUMBO_TAG_AUDIO
        /// \todo GUMBO_TAG_SOURCE

        auto item = createResourceItem(node->v.element, url);
        if (item) {
            auto contentModel = model->contentModel();
            contentModel->add(item);
        }
    }

    auto children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        auto childNode = static_cast<GumboNode*>(children->data[i]);
        searchForLinks(childNode, model, url);
    }
}

/*
 * See example:
 * https://github.com/google/gumbo-parser/blob/master/examples/find_links.cc
 */
void HtmlParser::parse(const QByteArray &bytes, const QUrl &url, Model *model)
{
    Q_ASSERT(model);
    auto output = gumbo_parse(bytes.constData());
    searchForLinks(output->root, model, url);
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}
