import { Component, For } from "solid-js";
import { useTextContext } from "~/contexts/text-context";
import { TextListItemType } from "~/types";
import TextListItem from "../text-list-item";

interface TextListContentProps {
  texts: TextListItemType[];
};

const TextListContent: Component<TextListContentProps> = (props) => {
  const { texts } = props;
  const { setSelectedTextId } = useTextContext();
  
  return (
    <For each={texts}>
      {(textListItem) => (
        <TextListItem onClick={() => setSelectedTextId(textListItem.id)}>
          {textListItem.title}
        </TextListItem>
      )}
    </For>
  );
};

export default TextListContent;