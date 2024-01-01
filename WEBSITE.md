# Github.io Website

The website is stored in `<path/to/project>/docs/`.

It uses [Jekyll](https://en.wikipedia.org/wiki/Jekyll_(software)) (a Ruby app) and the [Hideout](https://github.com/fongandrew/hydeout) theme.


## How to post

Add a *.md file under `<path/to/project>/docs/_posts`


## How to run the website locally

1. Install [Ruby](https://www.ruby-lang.org/en/downloads/).

2. Install the gems: [Jekyll](https://jekyllrb.com/) and [Bundler](https://bundler.io/). Open a console and type:

        > gem install jekyll
        > gem install bundler

3. Build the site and make it available on a local server:

        > cd <path/to/project>/docs/

4. Run

        > bundle exec jekyll serve

The site is available at [localhost:4000/ArrowDL](http://localhost:4000/ArrowDL/)


## How to upgrade Jekyll

To [upgrade Jekyll](https://jekyllrb.com/docs/upgrading/), open a console and type:

    > bundle update jekyll

    or alternatively:
    > bundle update


## How to clean

To clean the directory `<path/to/project>/docs/`:

    > bundle exec jekyll clean

To clean Jekyll:

    > bundle clean --force


## How to upgrade the dependencies (Hideout...)

1. Unzip last release in [Hideout](https://github.com/fongandrew/hydeout) to `/docs/`

2. Revert modification with Git
 
3. Eventually upgrade with Bundle

