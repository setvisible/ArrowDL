# Gihub.io Website

The pages are in /docs/

It uses [Jekyll](https://en.wikipedia.org/wiki/Jekyll_(software)) (a Ruby app) and the [Hideout](https://github.com/fongandrew/hydeout) theme.

## How to add a post

Add a *.md file under `/docs/_posts`


## How to run the website locally

1. Install Ruby

2. Install Jekyll and Bundler gems

    > gem install jekyll
    > gem install bundler

3. Build the site and make it available on a local server

    > cd /docs/
    > bundle exec jekyll serve

4. Browse to http://localhost:4000/DownZemAll/


## How to upgrade Jekyll

    > bundle update jekyll

or simply

    > bundle update

Source: https://jekyllrb.com/docs/upgrading/


## How to clean

    > bundle clean --force


## How to upgrade the dependencies (Hideout...)

1. Unzip last release in [Hideout](https://github.com/fongandrew/hydeout) to `/docs/`

2. Revert modification with Git
 
3. Eventually upgrade with Bundle

