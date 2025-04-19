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
  annotations: Annotation[];
  audio: string | null;
  id: number;
  language: string;
  text: string;
  text_object_id: number;
};

export type Annotation = {
  id: number;
  start: number;
  end: number;
  text_id: number;
};
