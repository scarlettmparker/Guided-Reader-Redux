import { TextListItem } from "~/types";

/**
 * Helper function to create mock text list data of N size
 * 
 * @param size Size of the mock data array
 * @return List of mock data
 */
export function createMockTextList(size: number): TextListItem[] {
  let mockData = [];
  let i;

  for (i = 0; i < size; i++) {
    mockData[i] = {
      id: i,
      title: "Text " + i
    };
  }

  return mockData;
}