import { Component } from "solid-js";
import { createMockTextList } from "~/utils/text-list";
import TextListItem from "~/components/text-list-item";

interface TextListItemsProps {
  mockData: ReturnType<typeof createMockTextList>;
  onSelect: (id: number) => void;
}

const TextListItems: Component<TextListItemsProps> = (props) => {
  return (
    <>
      {props.mockData.map((textListItem) => (
        <TextListItem onClick={() => props.onSelect(textListItem.id)}>
          {textListItem.title}
        </TextListItem>
      ))}
    </>
  );
};

export default TextListItems;