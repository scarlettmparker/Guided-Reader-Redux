export type TextListItemMockType = {
  id: number;
  title: string;
};

export type TextListItemType = {
  group_id: number;
  id: number;
  level: string;
  title: string;
};

export type TextType = {
  audio: string | null;
  id: number;
  language: string;
  text: string;
  text_object_id: number;
};
