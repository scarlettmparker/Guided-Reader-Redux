export type TextListItemMock = {
  id: number;
  title: string;
};

export type TextListItem = {
  group_id: number;
  id: number;
  level: string;
  title: string;
};

export type Text = {
  annotations: Annotation[];
  audio: string | null;
  id: number;
  language: string;
  text: string;
  text_object_id: number;
};

export type Author = {
  avatar: string;
  discord_id: string;
  discord_status: boolean;
  id: number;
  username: string;
};

export type Annotation = {
  id: number;
  start: number;
  end: number;
  text_id: number;
};

export type AnnotationResponse = {
  annotation: Annotation;
  author: Author;
  created_at: number;
  description: string;
  dislikes: number;
  likes: number;
};

export type UserData  = {
  annotation_count: number;
  dislike_count: number;
  levels: string[];
  like_count: number;
  user: UserDetails;
}

type UserDetails = {
  avatar: string;
  discord_id: string;
  discord_status: boolean;
  id: number;
  nickname: string;
  username: string;
};