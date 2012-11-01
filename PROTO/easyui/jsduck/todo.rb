require "jsduck/meta_tag"
module JsDuck::Tag
  # Implementation of @deferred tag
  class TodoTag < JsDuck::MetaTag
    def initialize
      @name = "todo"
      @key = :todo
      @signature = {:long => "todo", :short => "TODO"}
    end

    def to_html(v)
      <<-EOHTML
        <p>TODO: <b>#{v}</b></p>
      EOHTML
    end
  end
end
